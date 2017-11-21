#ifndef __TAGINTERFACE_H__
#define __TAGINTERFACE_H__

#include "ByteBuffer.h"
#include <functional>
#include <cstdint>

typedef std::function<int16_t(const BinaryData& packet)> TagWriteInterface_t;
typedef std::function<int16_t(BinaryData& packet)> TagReadInterface_t;

class TagInterface
{
public:
    TagInterface(const TagWriteInterface_t& wif, const TagReadInterface_t rif) : Write(wif), Read(rif) { }

    TagWriteInterface_t Write;
    TagReadInterface_t Read;
};

#endif
