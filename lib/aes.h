#pragma once

#include <cstdint>
#include <utility>
#include <cstddef>
namespace ncmdump {
class AES {

public:
    AES();
    template<size_t keylen>
    explicit AES(const uint8_t (&key)[keylen]) {
        setKey(key);
        keyExpansion();
    }

    virtual ~AES();
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
    uint8_t mKey[32];

    //the extended key,which can be 176bytes,208bytes,240bytes
    uint8_t mW[60][4];

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
}// namespace ncmdump