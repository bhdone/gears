#ifndef BHD_MINER_RPC_CLIENT_HPP
#define BHD_MINER_RPC_CLIENT_HPP

#include <curl/curl.h>
#include <plog/Log.h>

#include <json/value.h>
#include <json/reader.h>

#include <cstdint>

#include <string>
#include <string_view>

#include <type_traits>

#include "http_client.h"

#include "utils.h"
#include "types.h"

class Error : public std::runtime_error
{
public:
    explicit Error(char const* msg)
        : std::runtime_error(msg)
    {
    }
};

class NetError : public Error
{
public:
    explicit NetError(char const* msg)
        : Error(msg)
    {
    }
};

class RPCError : public Error
{
public:
    RPCError(int code, std::string msg)
        : Error(msg.c_str())
        , m_code(code)
        , m_msg(std::move(msg))
    {
    }

    int GetCode() const
    {
        return m_code;
    }

private:
    int m_code;
    std::string m_msg;
};

Json::Value MakeArg(std::string const& str);

Json::Value MakeArg(std::string_view str);

template <typename T> Json::Value MakeArg(T int_val, std::is_integral<T>&)
{
    return Json::Value(static_cast<Json::Int64>(int_val));
}

Json::Value MakeArg(Bytes const& data);

template <int N> Json::Value MakeArg(std::array<uint8_t, N> const& data)
{
    return MakeArg(MakeBytes(data));
}

class RPCClient
{
public:
    struct Result {
        Json::Value result;
        std::string error;
        int id;
    };

    RPCClient(bool no_proxy, std::string url, std::string const& cookie_path_str = "");

    RPCClient(bool no_proxy, std::string url, std::string user, std::string passwd);

    template <typename... T> Result Call(std::string const& method_name, T&&... vals)
    {
        Json::Value params;
        MakeParams(params, std::forward<T>(vals)...);
        return SendMethod(m_no_proxy, method_name, params);
    }

private:
    void MakeParams(Json::Value const&) { }

    template <typename T, typename... Ts> void MakeParams(Json::Value& out_params, T&& val, Ts&&... vals)
    {
        out_params.append(MakeArg(std::forward<T>(val)));
        MakeParams(out_params, std::forward<Ts>(vals)...);
    }

    Result SendMethod(bool no_proxy, std::string const& method_name, Json::Value const& params);

private:
    bool m_no_proxy;
    std::string m_url;
    std::string m_user;
    std::string m_passwd;
};

#endif
