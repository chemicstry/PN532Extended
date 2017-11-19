#include "PN532Extended.h"
#include "Utils.h"

#include "PN532.h"

CardType_t PN532Extended::IdentifyTypeACard(const TargetDataTypeA& tgdata)
{
    if (tgdata.SAK == 0x09 && tgdata.ATQA[0] == 0x00 && tgdata.ATQA[1] == 0x04)
        return CARD_TYPE_MIFARE_MINI;
    else if (tgdata.SAK == 0x08 && tgdata.ATQA[0] == 0x00 && tgdata.ATQA[1] == 0x04)
        return CARD_TYPE_MIFARE_CLASSIC_1K;
    else if (tgdata.SAK == 0x18 && tgdata.ATQA[0] == 0x00 && tgdata.ATQA[1] == 0x02)
        return CARD_TYPE_MIFARE_CLASSIC_4K;
    else if (tgdata.SAK == 0x00 && tgdata.ATQA[0] == 0x00 && tgdata.ATQA[1] == 0x44)
        return CARD_TYPE_MIFARE_ULTRALIGHT;
    else if (tgdata.SAK == 0x20 && tgdata.ATQA[0] == 0x03 && tgdata.ATQA[1] == 0x44)
        return CARD_TYPE_MIFARE_DESFIRE;
    else
        return CARD_TYPE_MAX;
}

PN532Extended::PN532Extended(PN532Interface& interface): _interface(interface)
{

}

void PN532Extended::begin()
{
    // Initialize HAL
    _interface.begin();
    _interface.wakeup();
}

int16_t PN532Extended::WriteCommand(const BinaryData& packet)
{
    #if PN532EXTENDED_DEBUG
    Serial.print("PN532Extended Write: ");
    PrintBin(packet);
    #endif

    // Call HAL
    return _interface.writeCommand(packet.data(), packet.size());
}

int16_t PN532Extended::ReadResponse(BinaryData& packet, uint16_t timeout)
{
    // Allocate space for data
    packet.resize(PN532_MAX_PACKET_SIZE);

    // Call HAL
    int16_t status = _interface.readResponse(packet.data(), PN532_MAX_PACKET_SIZE, timeout);

    #if PN532EXTENDED_DEBUG
    Serial.print("PN532Extended Read: ");
    PrintBin(packet);
    #endif

    // If successful, resize vector to real size
    if (status > 0)
        packet.resize(status);

    return status;
}

// Creates external card interface for communications
CardInterface PN532Extended::CreateCardInterface(uint8_t tg)
{
    return CardInterface(
        [tg, this](const BinaryData& packet) {
            // Build data exchange packet
            ByteBuffer buf;
            buf << COMMAND_INDATAEXCHANGE;
            buf << tg; // Target id
            buf << packet;
            return WriteCommand(buf.Data());
        },
        [tg, this](BinaryData& packet) {
            int16_t status = ReadResponse(packet);
            packet.erase(packet.begin()); // Remove status field. Inefficient. Need better way.
            return status; 
        }
    );
}

InListPassiveTargetResponse PN532Extended::InListPassiveTarget(uint8_t maxTargets, BrTy_t brty)
{
    // Initialise response struct
    InListPassiveTargetResponse resp;
    resp.NbTg = 0;

    // Initialise request struct
    InListPassiveTargetRequest req;
    req.MaxTg = maxTargets;
    req.BrTy = brty;

    // Serialize request
    ByteBuffer buf;
    buf << COMMAND_INLISTPASSIVETARGET;
    buf << req;
    
    if (WriteCommand(buf.Data()))
        return resp; // Return empty response if failed

    // Reuse request buffer for response
    buf.Clear();

    if (ReadResponse(buf.Data()) < 0)
        return resp; // Return empty response if failed

    // Deserialize data
    buf >> resp;

    return resp;
}
