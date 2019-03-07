#pragma once

#include <cstdint>
#include <utility>
#include <string>
#include <vector>
#include <cstring>
namespace ncmdump {
namespace aes {
class AES {
public:
    AES();
    template<size_t keylen>
    explicit AES(const uint8_t (&key)[keylen]) {
        setKey(key);
        keyExpansion();
    }

    virtual ~AES() = default;
    void encrypt_block(const uint8_t (&data)[16], uint8_t (&out)[16]);
    void decrypt_block(const uint8_t (&data)[16], uint8_t (&out)[16]);

private:
    //Number of columns
    int mNb{};

    //word length of the secret key used in one turn 
    int mNk{};

    //number of turns
    int mNr{};

    //the secret key,which can be 16bytes，24bytes or 32bytes
    uint8_t mKey[32]{};

    //the extended key,which can be 176bytes,208bytes,240bytes
    uint8_t mW[60][4]{};

    static const uint8_t sBox[];
    static const uint8_t invSBox[];
    //constant 
    static uint8_t rcon[];
    void setKey(const uint8_t (&key)[16]);
    void setKey(const uint8_t (&key)[24]);
    void setKey(const uint8_t (&key)[32]);

    void subBytes(uint8_t state[][4]);
    void shiftRows(uint8_t state[][4]);
    void mixColumns(uint8_t state[][4]);
    void addRoundKey(uint8_t state[][4], uint8_t w[][4]);

    void invSubBytes(uint8_t state[][4]);
    void invShiftRows(uint8_t state[][4]);
    void invMixColumns(uint8_t state[][4]);

    void keyExpansion();

    //
    uint8_t GF28Multi(uint8_t s, uint8_t a);

    void rotWord(uint8_t w[]);
    void subWord(uint8_t w[]);

    //get the secret key
    void getKeyAt(uint8_t key[][4], int i);

};
template<size_t keylen>
std::vector<uint8_t> ecb_decrypt(const uint8_t (&key)[keylen], const uint8_t *src, size_t len) {
    size_t bc, i;
    bc = len / 16;
    std::vector<uint8_t> ret;
    ret.reserve(len + 16);
    AES aes(key);
    const uint8_t *in = src;

    for (i = 0; i < bc - 1; i++) {
        ret.resize(ret.size() + 16);
        uint8_t *out = &ret.back() - 15;
        const uint8_t (&inblock)[16] = *reinterpret_cast<const uint8_t (*)[16]> (in);
        uint8_t (&outblock)[16] = *reinterpret_cast<uint8_t (*)[16]>(out);
        aes.decrypt_block(inblock, outblock);
        in += 16;
    }
    uint8_t data[16];
    aes.decrypt_block(*reinterpret_cast<const uint8_t (*)[16]> (in), data);
    uint8_t pad = data[15];
    if (pad > 0 & pad <= 16) {
        pad = static_cast<uint8_t>(16 - pad);
        ret.resize(ret.size() + pad);
        memcpy(&ret.back() - pad + 1, data, pad);
    }
    return ret;
}

}// namespace aes
}// namespace ncmdump