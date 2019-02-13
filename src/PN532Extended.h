#ifndef __PN532EXTENDED_H__
#define __PN532EXTENDED_H__

#include <cstdint>
#include "PN532Interface.h"
#include "PN532Packets.h"
#include "TagInterface.h"

#define PN532EXTENDED_DEBUG 0

using namespace PN532Packets;

enum CardType_t : uint8_t
{
    CARD_TYPE_MIFARE_MINI,
    CARD_TYPE_MIFARE_CLASSIC_1K,
    CARD_TYPE_MIFARE_CLASSIC_4K,
    CARD_TYPE_MIFARE_ULTRALIGHT,
    CARD_TYPE_MIFARE_DESFIRE,
    CARD_TYPE_MAX
};

#define PN532_MAX_PACKET_SIZE 255
#define PN532_DEFAULT_TIMEOUT 1000

class PN532Extended
{
public:
    static CardType_t IdentifyTypeACard(const TargetDataTypeA& tgdata);

    PN532Extended(PN532Interface& interface);
    void begin();

    int16_t WriteCommand(const BinaryData& packet);
    int16_t ReadResponse(BinaryData& packet, uint16_t timeout = PN532_DEFAULT_TIMEOUT);

    TagInterface CreateTagInterface(uint8_t tg);
    bool SetPassiveActivationRetries(uint8_t maxRetries);
    bool SAMConfig(SAMModes mode = SAM_MODE_NORMAL, uint8_t timeout = 20, uint8_t IRQ = 0x01);
    bool GetFirmwareVersion(GetFirmwareVersionResponse& resp);
    bool InListPassiveTarget(InListPassiveTargetResponse& resp, uint8_t maxTargets = 1, BrTy_t brty = BRTY_106KBPS_TYPE_A);
    uint8_t InRelease(uint8_t tg);

private:
    PN532Interface& _interface;
};

#endif
