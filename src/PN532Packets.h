#ifndef __PN532PACKETS_H__
#define __PN532PACKETS_H__

#include <cstdint>
#include <vector>
#include "ByteBuffer.h"

namespace PN532Packets
{
    enum Commands : uint8_t
    {
        COMMAND_INDATAEXCHANGE          = 0x40,
        COMMAND_INLISTPASSIVETARGET     = 0x4A
    };

    enum BrTy_t : uint8_t // Baud rate and modulation type
    {
        BRTY_106KBPS_TYPE_A     = 0x00, // ISO/IEC14443 Type A
        BRTY_212KBPS            = 0x01, // FeliCa polling
        BRTY_424KBPS            = 0x02, // FeliCa polling
        BRTY_106KBPS_TYPE_B     = 0x03, // ISO/IEC14443-3B
        BRTY_106KBPS_JEWEL      = 0x04  // Innovision Jewel tag
    };

    struct TargetDataTypeA
    {
        uint8_t Tg;
        uint8_t ATQA[2];
        uint8_t SAK;
        BinaryData UID;
        BinaryData ATS;
    };

    struct InListPassiveTargetRequest
    {
        uint8_t MaxTg;              // Maximum number of targets to be initialized by the PN532 (limit 2)
        BrTy_t BrTy;                // Baud rate and the modulation type to be used during the initialization
        BinaryData InitiatorData;   // Contents depend on BrTy value
    };

    struct InListPassiveTargetResponse
    {
        uint8_t NbTg;                   // Number of initialized targets
        BinaryData TgData; // Target Data
    };

}

ByteBuffer& operator>>(ByteBuffer& a, PN532Packets::TargetDataTypeA& b);
ByteBuffer& operator<<(ByteBuffer& a, const PN532Packets::InListPassiveTargetRequest& b);
ByteBuffer& operator>>(ByteBuffer& a, PN532Packets::InListPassiveTargetResponse& b);

#endif
