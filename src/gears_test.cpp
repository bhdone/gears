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
    EXPECT_TRUE(arg.isString());
    EXPECT_EQ(BytesToHex(src), arg.asString());
}

TEST(MakeArgs, Array)
{
    int const LEN = 16;
    std::array<uint8_t, LEN> src;
    for (int i = 0; i < src.size(); ++i) {
        src[i] = i;
    }
    auto arg = MakeArg<LEN>(src);
    EXPECT_TRUE(arg.isString());
    EXPECT_EQ(BytesToHex(MakeBytes(src)), arg.asString());
}
