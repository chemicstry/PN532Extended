#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include "ByteBuffer.h"

BinaryData AES_CBC_Decrypt(const BinaryData& data, const BinaryData& key, BinaryData& iv);
BinaryData AES_CBC_Encrypt(const BinaryData& data, const BinaryData& key, BinaryData& iv);

#endif
