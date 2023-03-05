#include "rpc_client.h"

#include <curl/curl.h>

#include <fstream>
#include <iostream>

#include <filesystem>
namespace fs = std::filesystem;

Json::Value MakeArg(std::string const& str)
{
    return Json::Value(static_cast<Json::String>(str));
}

Json::Value MakeArg(std::string_view str)
{
    return Json::Value(static_cast<Json::String>(str));
}

Json::Value MakeArg(Bytes const& data)
{
    return Json::Value(BytesToHex(data));
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

RPCClient::Result RPCClient::SendMethod(bool no_proxy, std::string const& method_name, Json::Value const& params)
{
    Json::Value root;
    root["jsonrpc"] = "2.0";
    root["method"] = method_name;
    root["params"] = params;
    // Invoke curl
    HTTPClient client(m_url, m_user, m_passwd, no_proxy);
    std::string send_str = root.toStyledString();
    PLOG_DEBUG << "sending: `" << send_str << "`";
    int code;
    std::string err_str;
    std::tie(code, err_str) = client.Send(send_str);
    if (code != CURLE_OK) {
        std::stringstream ss;
        ss << "RPC command error `" << method_name << "`: " << err_str;
        throw NetError(ss.str().c_str());
    }
    // Analyze the result
    Bytes received_data = client.GetReceivedData();
    if (received_data.empty()) {
        throw NetError("empty result from RPC server");
    }
    char const* psz = reinterpret_cast<char const*>(received_data.data());
    Json::Value res;
    Json::CharReaderBuilder builder;
    std::shared_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errs;
    if (!reader->parse(psz, psz + strlen(psz), &res, &errs)) {
        throw Error("cannot parse the result from rpc server");
    }
    // Build result and return
    PLOG_DEBUG << "received: `" << res.toStyledString() << "`";
    Result result;
    if (res.isMember("result")) {
        result.result = res["result"];
    }
    if (res.isMember("error") && !res["error"].isNull()) {
        Json::Value errorJson = res["error"];
        int code = errorJson["code"].asInt();
        std::string msg = errorJson["message"].asString();
        throw RPCError(code, msg);
    }
    if (res.isMember("id") && res["id"].isInt()) {
        result.id = res["id"].asInt();
    }
    return result;
}
