#include "utils.h"

#include <sstream>
#include <regex>
#include <algorithm>

char Byte4bToHexChar(uint8_t hex) { return hex_chars[hex]; }

std::string ByteToHex(uint8_t byte) {
    std::string hex(2, '0');
    uint8_t hi = (byte & 0xf0) >> 4;
    uint8_t lo = byte & 0x0f;
    hex[0] = Byte4bToHexChar(hi);
    hex[1] = Byte4bToHexChar(lo);
    return hex;
}

std::string BytesToHex(Bytes const& bytes) {
    std::stringstream ss;
    for (uint8_t byte : bytes) {
        ss << ByteToHex(byte);
    }
    return ss.str();
}

std::string TrimLeftString(std::string const& str) {
    auto p = str.find_first_not_of(' ');
    if (p == std::string::npos) {
        return "";
    }
    return str.substr(p);
}

bool Expand1EnvPath(std::string const& path, std::string& out_expanded) {
    std::regex r("\\$\\w+");
    std::sregex_iterator begin(std::begin(path), std::end(path), r), end;
    if (begin != end) {
        auto const& m = *begin;
        out_expanded = path;
        // erase the env
        auto erase_begin = std::begin(out_expanded) + m.position();
        auto erase_end = erase_begin + m.length();
        out_expanded.erase(erase_begin, erase_end);
        // insert the actual value
        char* actual = getenv(m.str().c_str() + 1);
        out_expanded.insert(m.position(), actual);
        // we got a match and replacement has been made
        return true;
    }
    return false;
}

std::string ExpandEnvPath(std::string const& path) {
    std::string src = path;
    std::string dst = path;
    while (Expand1EnvPath(src, dst)) {
        src = dst;
    }
    return dst;
}

std::string ToLowerCase(std::string const& str) {
    std::string res;
    std::transform(std::cbegin(str), std::cend(str), std::back_inserter(res), [](char ch) {
        return std::tolower(ch);
    });
    return res;
}
