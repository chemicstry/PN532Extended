#include "PN532Extended.h"
#include "Utils.h"

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

// Creates external tag interface for communications
TagInterface PN532Extended::CreateTagInterface(uint8_t tg)
{
    return TagInterface(
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

bool PN532Extended::GetFirmwareVersion(GetFirmwareVersionResponse& resp)
{
    // Serialize request
    ByteBuffer buf;
    buf << COMMAND_GETFIRMWAREVERSION;

    if (WriteCommand(buf.Data()))
        return false;

    // Reuse request buffer for response
    buf.Clear();

    if (ReadResponse(buf.Data()) < 0)
        return false;

    // Deserialize data
    buf >> resp;

    return true;
}

bool PN532Extended::SAMConfig(SAMModes mode, uint8_t timeout, uint8_t IRQ)
{
    SAMConfiguration req;
    req.Mode = mode;
    req.Timeout = timeout;
    req.IRQ = IRQ;

    // Serialize request
    ByteBuffer buf;
    buf << COMMAND_SAMCONFIGURATION;
    buf << req;

    if (WriteCommand(buf.Data()))
        return false;

    // Reuse request buffer for response
    buf.Clear();

    if (ReadResponse(buf.Data()) < 0)
        return false;

    return true;
}

bool PN532Extended::SetPassiveActivationRetries(uint8_t maxRetries)
{
    // Max retries options
    RFConfiguration_MaxRetries req;
    req.MxRtyATR = 0xFF;
    req.MxRtyPSL = 0x01;
    req.MxRtyPassiveActivation = maxRetries;

    // Serialize request
    ByteBuffer buf;
    buf << COMMAND_GETFIRMWAREVERSION;
    buf << req;

    if (WriteCommand(buf.Data()))
        return false;

    // Reuse request buffer for response
    buf.Clear();

    if (ReadResponse(buf.Data()) < 0)
        return false;

    return true;
}

bool PN532Extended::InListPassiveTarget(InListPassiveTargetResponse &resp, uint8_t maxTargets, BrTy_t brty)
{
    // Initialise response struct
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
        return false;

    // Reuse request buffer for response
    buf.Clear();

    if (ReadResponse(buf.Data()) < 0)
        return false;

    // Deserialize data
    buf >> resp;

    return true;
}
