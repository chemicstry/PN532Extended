#include "PN532Packets.h"

using namespace PN532Packets;

ByteBuffer& operator>>(ByteBuffer& a, TargetDataTypeA& b)
{
    a >> b.Tg;
    a >> b.ATQA[0];
    a >> b.ATQA[1];

    a >> b.SAK;

    uint8_t UIDLen = 0;
    a >> UIDLen;
    b.UID = a.ReadBinary(UIDLen);

    uint8_t ATSLen = 0;
    a >> ATSLen;
    if (ATSLen > 1)
        b.ATS = a.ReadBinary(ATSLen-1); // ATS len includes "ATSLen" byte

    return a;
}

ByteBuffer& operator<<(ByteBuffer& a, const InListPassiveTargetRequest& b)
{
    a << b.MaxTg;
    a << b.BrTy;
    a << b.InitiatorData;

    return a;
}

ByteBuffer& operator>>(ByteBuffer& a, InListPassiveTargetResponse& b)
{
    a >> b.NbTg;
    b.TgData = a.ReadBinary(); // Read till end

    return a;
}
