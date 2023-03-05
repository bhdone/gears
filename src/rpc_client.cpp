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

RPCClient::RPCClient(bool no_proxy, std::string url, std::string const& cookie_path_str)
    : m_no_proxy(no_proxy)
    , m_url(std::move(url))
{
    if (cookie_path_str.empty()) {
        throw std::runtime_error("cookie is empty, cannot connect to btchd core");
    }

    LoadCookie(cookie_path_str);
}

RPCClient::RPCClient(bool no_proxy, std::string url, std::string user, std::string passwd)
    : m_no_proxy(no_proxy)
    , m_url(std::move(url))
    , m_user(std::move(user))
    , m_passwd(std::move(passwd))
{
}

void RPCClient::LoadCookie(std::string_view cookie_path_str)
{
    fs::path cookie_path(cookie_path_str);
    std::ifstream cookie_reader(cookie_path.string());
    if (!cookie_reader.is_open()) {
        std::stringstream ss;
        ss << "cannot open to read " << cookie_path;
        throw std::runtime_error(ss.str());
    }
    std::string auth_str;
    std::getline(cookie_reader, auth_str);
    if (auth_str.empty()) {
        throw std::runtime_error("cannot read auth string from `.cookie`");
    }
    auto pos = auth_str.find_first_of(':');
    std::string user_str = auth_str.substr(0, pos);
    std::string passwd_str = auth_str.substr(pos + 1);
    m_user = std::move(user_str);
    m_passwd = std::move(passwd_str);
}

RPCClient::Result RPCClient::SendMethod(bool no_proxy, std::string const& method_name, UniValue const& params)
{
    UniValue root(UniValue::VOBJ);
    root.pushKV("jsonrpc", "2.0");
    root.pushKV("method", method_name);
    root.pushKV("params", params);
    // Invoke curl
    HTTPClient client(m_url, m_user, m_passwd, no_proxy);
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
