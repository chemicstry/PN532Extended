#ifndef __CARDINTERFACE_H__
#define __CARDINTERFACE_H__

#include "ByteBuffer.h"
#include <functional>
#include <cstdint>

typedef std::function<int16_t(const BinaryData& packet)> CardWriteInterface_t;
typedef std::function<int16_t(BinaryData& packet)> CardReadInterface_t;

class CardInterface
{
public:
    CardInterface(const CardWriteInterface_t& wif, const CardReadInterface_t rif) : Write(wif), Read(rif) { }

    CardWriteInterface_t Write;
    CardReadInterface_t Read;
};

#endif
