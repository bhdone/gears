#include <gtest/gtest.h>

#include <array>

#include "utils.h"

#include "rpc_client.h"

TEST(Utils, BytesToHex)
{
    Bytes src(16, '\0');
    for (int i = 0; i < src.size(); ++i) {
        src[i] = i;
    }
    std::string res = BytesToHex(src);
    EXPECT_EQ(res, "000102030405060708090a0b0c0d0e0f");
}

TEST(Utils, MakeBytes)
{
    std::array<uint8_t, 16> src;
    for (int i = 0; i < src.size(); ++i) {
        src[i] = i;
    }
    Bytes dst = MakeBytes(src);
    EXPECT_EQ(BytesToHex(dst), "000102030405060708090a0b0c0d0e0f");
}

TEST(Utils, TrimLeftString)
{
    std::string_view src = "    hello world";
    EXPECT_EQ(TrimLeftString(std::string(src)), "hello world");
}

TEST(Utils, ExpandEnvPath)
{
    std::string_view postfix = "/workspace/folder";
    std::string home = getenv("HOME");
    EXPECT_EQ(ExpandEnvPath("$HOME/workspace/folder"), home + std::string(postfix));
}

TEST(Utils, ToLowerCase)
{
    EXPECT_EQ(ToLowerCase("aAaBbBZzZ"), "aaabbbzzz");
}

TEST(MakeArg, Bytes)
{
    Bytes src(10, '\0');
    auto arg = MakeArg(src);
    EXPECT_TRUE(arg.isStr());
    EXPECT_EQ(BytesToHex(src), arg.get_str());
}

TEST(MakeArgs, Array)
{
    int const LEN = 16;
    std::array<uint8_t, LEN> src;
    for (int i = 0; i < src.size(); ++i) {
        src[i] = i;
    }
    auto arg = MakeArg<LEN>(src);
    EXPECT_TRUE(arg.isStr());
    EXPECT_EQ(BytesToHex(MakeBytes(src)), arg.get_str());
}

char const* SZ_RPC_URL = "http://127.0.0.1:18732";
char const* SZ_RPC_COOKIE_PATH = "$HOME/.btchd/testnet3/.cookie";

class RPCTest : public testing::Test
{
protected:
    void SetUp() override
    {
        rpc_ = std::make_unique<RPCClient>(true, SZ_RPC_URL, RPCLogin(ExpandEnvPath(SZ_RPC_COOKIE_PATH)));
    }

    void TearDown() override { }

    std::unique_ptr<RPCClient> rpc_;
};

TEST_F(RPCTest, Connect)
{
    EXPECT_NO_THROW({ rpc_->Call("querychallenge"); });
}
