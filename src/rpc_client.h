#ifndef BHD_MINER_RPC_CLIENT_HPP
#define BHD_MINER_RPC_CLIENT_HPP

#include <univalue.h>

#include <cstdint>

#include <stdexcept>

#include <string>
#include <string_view>

#include <type_traits>

#include "http_client.h"
#include "rpc_login.hpp"

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

UniValue MakeArg(std::string const& str);

UniValue MakeArg(std::string_view str);

UniValue MakeArg(int64_t val);

UniValue MakeArg(Bytes const& data);

template <int N> UniValue MakeArg(std::array<uint8_t, N> const& data)
{
    return MakeArg(MakeBytes(data));
}

class RPCClient
{
public:
    struct Result {
        UniValue result;
        std::string error;
        int id;
    };

    RPCClient(bool no_proxy, std::string url, RPCLogin login);

    template <typename... T> Result Call(std::string const& method_name, T&&... vals)
    {
        UniValue params(UniValue::VARR);
        MakeParams(params, std::forward<T>(vals)...);
        return SendMethod(m_no_proxy, method_name, params);
    }

private:
    void MakeParams(UniValue const&) { }

    template <typename T, typename... Ts> void MakeParams(UniValue& out_params, T&& val, Ts&&... vals)
    {
        out_params.push_back(MakeArg(std::forward<T>(val)));
        MakeParams(out_params, std::forward<Ts>(vals)...);
    }

    Result SendMethod(bool no_proxy, std::string const& method_name, UniValue const& params);

private:
    bool m_no_proxy;
    std::string m_url;
    RPCLogin m_login;
};

#endif
