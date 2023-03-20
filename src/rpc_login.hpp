#ifndef RPCLOGIN_HPP
#define RPCLOGIN_HPP

#include <string>

#include <fstream>
#include <optional>

#include <filesystem>
namespace fs = std::filesystem;

#include <tinyformat.h>

class RPCLogin
{
public:
    enum class Type { LOGIN_WITH_COOKIE, LOGIN_WITH_RAW_PASSWD };

    explicit RPCLogin(std::string cookie_path_str)
        : type_(Type::LOGIN_WITH_COOKIE)
        , cookie_path_str_(std::move(cookie_path_str))
    {
        ReloadFromCookie();
    }

    RPCLogin(std::string user, std::string passwd)
        : type_(Type::LOGIN_WITH_RAW_PASSWD)
        , valid_(true)
        , user_(std::move(user))
        , passwd_(std::move(passwd))
    {
    }

    void SetInvalid()
    {
        valid_ = false;
    }

    bool Valid() const
    {
        return valid_;
    }

    void ReloadFromCookie()
    {
        if (type_ == Type::LOGIN_WITH_COOKIE) {
            valid_ = false;
            if (fs::exists(cookie_path_str_) && fs::is_regular_file(cookie_path_str_)) {
                // fs::path cookie_path(cookie_path_str_);
                std::ifstream cookie_reader;
                cookie_reader.exceptions(cookie_reader.exceptions() | std::ios::failbit);
                try {
                    cookie_reader.open(cookie_path_str_);
                } catch (std::ios_base::failure const& e) {
                    throw std::runtime_error(
                            tinyformat::format("failed to open file %s, err: %s", cookie_path_str_, e.what()));
                }
                std::string auth_str;
                std::getline(cookie_reader, auth_str);
                if (auth_str.empty()) {
                    throw std::runtime_error("cannot read auth string from `.cookie`");
                }
                auto pos = auth_str.find_first_of(':');
                user_ = auth_str.substr(0, pos);
                passwd_ = auth_str.substr(pos + 1);
                valid_ = true;
            }
        }
    }

    Type GetType() const
    {
        return type_;
    }

    std::pair<std::string, std::string> GetUserPasswd() const
    {
        return std::make_pair(user_, passwd_);
    }

private:
    Type type_;
    std::string cookie_path_str_;
    bool valid_ { false };
    std::string user_;
    std::string passwd_;
};

#endif
