#ifndef __PN532PACKETS_H__
#define __PN532PACKETS_H__

#include <cstdint>
#include <vector>
#include "ByteBuffer.h"

namespace PN532Packets
{
    enum Commands : uint8_t
    {
        COMMAND_GETFIRMWAREVERSION      = 0x02,
        COMMAND_SAMCONFIGURATION        = 0x14,
        COMMAND_RFCONFIGURATION         = 0x32,
        COMMAND_INDATAEXCHANGE          = 0x40,
        COMMAND_INLISTPASSIVETARGET     = 0x4A
    };

    struct GetFirmwareVersionResponse
    {
        uint8_t IC;     // IC version (PN532 is 0x32)
        uint8_t Ver;    // Firmware version
        uint8_t Rev;    // Firmware revision
        struct Support_t {
            uint8_t ISO14443_TYPEA  : 1;
            uint8_t ISO14443_TYPEB  : 1;
            uint8_t ISO18092        : 1;
            uint8_t RFU             : 5;
        } Support;      // Supported functionalities
    };

    enum SAMModes : uint8_t
    {
        SAM_MODE_NORMAL         = 0x01, // the SAM is not used; this is the default mode
        SAM_MODE_VIRTUAL_CARD   = 0x02, // the couple PN532+SAM is seen as only one contactless SAM card from the external world
        SAM_MODE_WIRED_CARD     = 0x03, // the host controller can access to the SAM with standard PCD commands (InListPassiveTarget, InDataExchange, …)
        SAM_MODE_DUAL_CARD      = 0x04  // both the PN532 and the SAM are visible from the external world as two separated targets.
    };

    struct SAMConfiguration
    {
        SAMModes Mode;
        uint8_t Timeout;
        uint8_t IRQ;
    };

    struct RFConfiguration_MaxRetries
    {
        uint8_t MxRtyATR;
        uint8_t MxRtyPSL;
        uint8_t MxRtyPassiveActivation;
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

ByteBuffer& operator>>(ByteBuffer& a, PN532Packets::GetFirmwareVersionResponse& b);
ByteBuffer& operator<<(ByteBuffer& a, const PN532Packets::SAMConfiguration& b);
ByteBuffer& operator<<(ByteBuffer& a, const PN532Packets::RFConfiguration_MaxRetries& b);
ByteBuffer& operator>>(ByteBuffer& a, PN532Packets::TargetDataTypeA& b);
ByteBuffer& operator<<(ByteBuffer& a, const PN532Packets::InListPassiveTargetRequest& b);
ByteBuffer& operator>>(ByteBuffer& a, PN532Packets::InListPassiveTargetResponse& b);

#endif
