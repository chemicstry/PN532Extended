#ifndef __UTILS_H__
#define __UTILS_H__

#include "ByteBuffer.h"
#include "Arduino.h"

inline void PrintBin(const BinaryData& in)
{
    for (auto data : in)
    {
        Serial.print(data, HEX);
        Serial.print(' ');
    }
    Serial.print('\n');
}

inline void iso14443b_crc(uint8_t *pbtData, size_t szLen, uint8_t *pbtCrc)
{
    uint32_t wCrc = 0xFFFF;

    do {
        uint8_t  bt;
        bt = *pbtData++;
        bt = (bt ^ (uint8_t)(wCrc & 0x00FF));
        bt = (bt ^ (bt << 4));
        wCrc = (wCrc >> 8) ^ ((uint32_t) bt << 8) ^ ((uint32_t) bt << 3) ^ ((uint32_t) bt >> 4);
    } while (--szLen);
    wCrc = ~wCrc;
    *pbtCrc++ = (uint8_t)(wCrc & 0xFF);
    *pbtCrc = (uint8_t)((wCrc >> 8) & 0xFF);
}

inline void PadToBlocksize(BinaryData& data, size_t blocksize, uint8_t padding = 0x00)
{
    size_t remainder = data.size() % blocksize;

    if (remainder)
        data.resize(data.size() + blocksize - remainder, padding);
}

#endif