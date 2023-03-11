#include "rpc_client.h"

#include <curl/curl.h>
#include <tinyformat.h>

#include <fstream>

#include <filesystem>
namespace fs = std::filesystem;

UniValue MakeArg(std::string const& str)
{
    return UniValue(UniValue::VSTR, str);
}

UniValue MakeArg(std::string_view str)
{
    return UniValue(UniValue::VSTR, std::string(str));
}

UniValue MakeArg(Bytes const& data)
{
    return UniValue(UniValue::VSTR, BytesToHex(data));
}

RPCClient::RPCClient(bool no_proxy, std::string url, RPCLogin login)
    : m_no_proxy(no_proxy)
    , m_url(std::move(url))
    , m_login(std::move(login))
{
}

RPCClient::Result RPCClient::SendMethod(bool no_proxy, std::string const& method_name, UniValue const& params)
{
    UniValue root(UniValue::VOBJ);
    root.pushKV("jsonrpc", "2.0");
    root.pushKV("method", method_name);
    root.pushKV("params", params);
    // Retrieve user/passwd from rpc login
    auto plogin = m_login.GetUserPasswd();
    if (!plogin.has_value()) {
        throw NetError("cannot retrieve the user/passwd to connect the rpc service");
    }
    // Invoke curl
    HTTPClient client(m_url, plogin->first, plogin->second, no_proxy);
    std::string send_str = root.write();
    int code;
    std::string err_str;
    std::tie(code, err_str) = client.Send(send_str);
    if (code != CURLE_OK) {
        std::stringstream ss;
        ss << "RPC command error `" << method_name << "`: " << err_str;
        throw NetError(tinyformat::format("RPC command error `%s`: %s", method_name, err_str).c_str());
    }
    // Analyze the result
    Bytes received_data = client.GetReceivedData();
    if (received_data.empty()) {
        throw NetError("empty result from RPC server");
    }
    int n = received_data.size();
    received_data.resize(n + 1);
    received_data[n] = '\0';
    char const* psz = reinterpret_cast<char const*>(received_data.data());
    UniValue res;
    bool succ = res.read(psz);
    if (!succ) {
        throw Error("cannot parse the result from rpc server");
    }
    // Build result and return
    Result result;
    if (res.exists("result")) {
        result.result = res["result"];
    }
    res["error"].empty();
    if (res.exists("error") && !res["error"].isNull()) {
        UniValue errorJson = res["error"];
        int code = errorJson["code"].get_int();
        std::string msg = errorJson["message"].get_str();
        throw RPCError(code, msg);
    }
    if (res.exists("id") && res["id"].isNum()) {
        result.id = res["id"].get_int();
    }
    return result;
}
