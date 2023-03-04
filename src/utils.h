#ifndef UTILS_HPP
#define UTILS_HPP

#include <array>
#include <string>

#include "types.h"

char const hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

char Byte4bToHexChar(uint8_t hex);

std::string ByteToHex(uint8_t byte);

std::string BytesToHex(Bytes const& bytes);

template <size_t N>
Bytes MakeBytes(std::array<uint8_t, N> const& val) {
    Bytes res(N);
    memcpy(res.data(), val.data(), N);
    return res;
}

std::string TrimLeftString(std::string const& str);

bool Expand1EnvPath(std::string const& path, std::string& out_expanded);

std::string ExpandEnvPath(std::string const& path);

std::string ToLowerCase(std::string const& str);

#endif
