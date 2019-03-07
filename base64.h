#ifndef _MACARON_BASE64_H_
#define _MACARON_BASE64_H_

#include <string>
#include <vector>

static constexpr const char sEncodingTable[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

static constexpr const uint8_t kDecodingTable[] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

namespace ncmdump {
namespace base64 {

class EncodeError : public std::runtime_error {
public:
    EncodeError() : std::runtime_error("Base64 encode error:") {}
    explicit EncodeError(const char *s) : std::runtime_error(std::string("Base64 encode error:") + s) {}
    explicit EncodeError(const std::string &s) : std::runtime_error("Base64 encode error:" + s) {}
};
class DecodeError : public std::runtime_error {
public:
    DecodeError() : std::runtime_error("Base64 decode error:") {}
    explicit DecodeError(const char *s) : std::runtime_error(std::string("Base64 decode error:") + s) {}
    explicit DecodeError(const std::string &s) : std::runtime_error("Base64 decode error:" + s) {}
};

[[nodiscard]]
std::string encode(const uint8_t *data, size_t len);
[[nodiscard]]
inline std::string encode(const std::vector<uint8_t> &data) { return encode(data.data(), data.size()); }
[[nodiscard]]
std::vector<uint8_t> decode(const char *s, size_t len);
[[nodiscard]]
inline std::vector<uint8_t> decode(const std::string &s) { return decode(s.data(), s.size()); }
}// namespace ncmdump::base64

}// namespace ncmdump



std::string ncmdump::base64::encode(const uint8_t *data, size_t len) {
    std::string out;
    out.reserve(4 * (len + 2) / 3);
    auto p = std::back_inserter(out);
    size_t i = 0;
    for (; i < len; i += 3) {
        *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
        *p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((data[i + 1] & 0xF0) >> 4)];
        *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2) | ((data[i + 2] & 0xC0) >> 6)];
        *p++ = sEncodingTable[data[i + 2] & 0x3F];
    }
    if (i < len) {
        *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
        if (i == (len - 1)) {
            *p++ = sEncodingTable[((data[i] & 0x3) << 4)];
            *p++ = '=';
        } else {
            *p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((data[i + 1] & 0xF0) >> 4)];
            *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2)];
        }
        *p++ = '=';
    }
    return out;
}
std::vector<uint8_t> ncmdump::base64::decode(const char *s, size_t len) {
    if (len % 4) {
        throw DecodeError("Input data size is not a multiple of 4");
    }
    std::vector<uint8_t> out;
    out.reserve(len * 3 / 4);
    auto p = std::back_inserter(out);

    for (size_t i = 0; i < len;) {
        auto a = s[i] == '=' ? (0 & ++i) : kDecodingTable[s[i++]];
        auto b = s[i] == '=' ? (0 & ++i) : kDecodingTable[s[i++]];
        auto c = s[i] == '=' ? (0 & ++i) : kDecodingTable[s[i++]];
        auto d = s[i] == '=' ? (0 & ++i) : kDecodingTable[s[i++]];

        auto triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

        *p++ = (triple >> 2 * 8) & 0xFF;
        *p++ = (triple >> 1 * 8) & 0xFF;
        *p++ = (triple >> 0 * 8) & 0xFF;
    }
    return out;
}

#endif /* _MACARON_BASE64_H_ */
