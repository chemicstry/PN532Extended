#include "Desfire.h"
#include "Exception.h"
#include "Crypto.h"
#include "Utils.h"

ByteBuffer& operator<<(ByteBuffer& a, const ISO7816_4_CAPDU& b)
{
    a << b.CLA;
    a << b.INS;
    a << b.P1;
    a << b.P2;
    a << b.Lc;
    a << b.Data;
    a << b.Le;

    return a;
}

ByteBuffer& operator>>(ByteBuffer& a, ISO7816_4_RAPDU& b)
{
    b.Data = a.ReadBinary(a.RemainingSize()-2);
    a >> b.SW1;
    a >> b.SW2;

    return a;
}

Desfire::Desfire(TagInterface& interface) : _selectedApplication(0), _authenticatedKeyNo(-1), _interface(interface)
{

}

bool Desfire::Connect()
{
    ISO7816_4_CAPDU capdu;

    // Select file instruction
    capdu.CLA = ISO7816_4_CLA_WITHOUT_SM_LAST;
    capdu.INS = 0xA4;
    capdu.P1 = 0x04;
    capdu.P2 = 0x00;
    capdu.Data = DESFIRE_AID;
    capdu.Lc = capdu.Data.size();
    capdu.Le = 0x00;

    ByteBuffer buf;
    buf << capdu;

    // Send
    if (_interface.Write(buf.Data()))
        return false;

    // Reuse buffer
    buf.Clear();

    // Receive
    if (_interface.Read(buf.Data()) < 0)
        return false;

    ISO7816_4_RAPDU rapdu;
    buf >> rapdu;

    if (rapdu.SW1 == 0x90 && rapdu.SW2 == 0x00)
        return true;
    
    return false;
}

bool Desfire::Transceive(const DesfireInstruction_t ins, const BinaryData& in, BinaryData& out)
{
    ISO7816_4_CAPDU capdu;

    // Desfire instructions are wrapped in custom ISO7816-4 command class
    capdu.CLA = 0x90;
    capdu.INS = ins;
    capdu.P1 = 0x00;
    capdu.P2 = 0x00;
    capdu.Data = in;
    capdu.Lc = capdu.Data.size();
    capdu.Le = 0x00;

    ByteBuffer buf;
    buf << capdu;

    // Send
    if (_interface.Write(buf.Data()))
        return false;

    // Reuse buffer
    buf.Clear();

    // Receive
    if (_interface.Read(buf.Data()) < 0)
        return false;

    // Deserialize received packet
    ISO7816_4_RAPDU rapdu;
    buf >> rapdu;

    // Desfire error codes (DesfireStatus_t) are sent in SW2 variable
    _lastError = (DesfireStatus_t)rapdu.SW2;

    if (_lastError == DF_STATUS_OPERATION_OK || _lastError == DF_STATUS_ADDITIONAL_FRAME)
    {
        out = rapdu.Data;
        return true;
    }

    return false;
}

DesfireInstruction_t Desfire::GetAuthCmd(const DesfireKeyType_t& type)
{
    switch (type) {
        case DF_KEY_DES:
        case DF_KEY_3DES:
            return DF_INS_AUTHENTICATE_LEGACY;
        case DF_KEY_3K3DES:
            return DFEV1_INS_AUTHENTICATE_ISO;
        case DF_KEY_AES:
            return DFEV1_INS_AUTHENTICATE_AES;
    }

    return DF_INS_MAX;
}

bool Desfire::Authenticate(const uint8_t keyno, const DesfireKey& key)
{
    // Currently only AES is supported
    if (key.Type != DF_KEY_AES)
        return false;

    // Get Desfire instruction based on key type
    DesfireInstruction_t cmd = GetAuthCmd(key.Type);
    ByteBuffer args;
    args << keyno;

    // Transceive data. Card returns encrypted RndB value (randomly generated)
    BinaryData RndBEnc;
    if (!Transceive(cmd, args.Data(), RndBEnc))
        return false;

    // Size should be 16 bytes
    if (RndBEnc.size() != 16)
        return false;

    // Start off with zero IV. RndB is always a random value so this shouldn't be a security problem
    BinaryData IV(16, 0x00);

    // Decrypt RndB
    BinaryData RndB = AES_CBC_Decrypt(RndBEnc, key.Key, IV);

    // Rotate RndB
    BinaryData RndBRot;
    RndBRot.assign(RndB.begin()+1, RndB.end());
    RndBRot.push_back(RndB[0]);

    // Generate a random 16 byte value RndA
    BinaryData RndA;
    for (int i = 0; i < 16; ++i)
        RndA.push_back(rand() % 0xFF);

    // Build authentication token
    ByteBuffer Token;
    Token << RndA;
    Token << RndBRot;
    
    // Encrypt token and use RndBEnc as IV
    BinaryData TokenEnc = AES_CBC_Encrypt(Token.Data(), key.Key, IV);
    
    BinaryData RndARotEnc;
    if (!Transceive(DF_INS_ADDITIONAL_FRAME, TokenEnc, RndARotEnc))
        return false;

    // Decrypt RndARot
    BinaryData RndARot = AES_CBC_Decrypt(RndARotEnc, key.Key, IV);

    // Calculate a local RndARot value
    BinaryData RndARotLocal;
    RndARotLocal.assign(RndA.begin()+1, RndA.end());
    RndARotLocal.push_back(RndA[0]);

    // Check if final values match
    if (RndARotLocal == RndARot)
    {
        _authenticatedKeyNo = keyno;
        _sessionKey = CreateSessionKey(RndA, RndB, key);
        _sessionKeyIV = BinaryData(_sessionKey.Key.size(), 0x00);

        return true;
    }
    else
        return false;
}

bool Desfire::ChangeKey(uint8_t keyno, const DesfireKey& key)
{
    // Maximum keyno is 0x0F
    keyno &= 0x0F;

    // Can only change authenticated key
    if (_authenticatedKeyNo != keyno)
        return false;

    // Key type is encoded in keyno and can only be changed on master key
    if (_selectedApplication == 0)
    {
        switch (key.Type)
        {
            case DF_KEY_DES:
            case DF_KEY_3DES:
                break;
            case DF_KEY_3K3DES:
                keyno |= 0x40;
                break;
            case DF_KEY_AES:
                keyno |= 0x80;
                break;
        }
    }

    // New key is sent encrypted with session key. Cryptogram is data prepared for encryption
    ByteBuffer cryptogram;

    // Serialize key (8 byte keys are repeated)
    if (key.Type == DF_KEY_DES || key.Type == DF_KEY_3DES)
        cryptogram << key.Key << key.Key;
    else
        cryptogram << key.Key;
    
    // AES key has version. Just keep 0 ?
    if (key.Type == DF_KEY_AES)
        cryptogram.Append<uint8_t>(0x00);

    // Legacy authentication keys use ISO14443A CRC
    if (key.Type == DF_KEY_DES || key.Type == DF_KEY_3DES)
    {
        uint16_t crc;
        iso14443b_crc(cryptogram.Data().data()+2, cryptogram.Size()-2, (uint8_t*)&crc); // Do not include 
        cryptogram << crc;
    }
    else
    {
        // Desfire calculates crc also over command byte which is in completely different place
        uint32_t crc = 0xFFFFFFFF;
        desfire_crc32_byte(&crc, DF_INS_CHANGE_KEY);
        desfire_crc32_byte(&crc, keyno);

        // CRC cryptogram
        for (uint8_t val : cryptogram.Data())
            desfire_crc32_byte(&crc, val);

        // Append crc
        cryptogram << crc;
    }

    // Pad cryptogram to blocksize
    PadToBlocksize(cryptogram.Data(), key.Key.size());

    // Only AES for now
    BinaryData cryptogramEnc;
    if (key.Type == DF_KEY_AES)
        cryptogramEnc = AES_CBC_Encrypt(cryptogram.Data(), _sessionKey.Key, _sessionKeyIV);
    else
        return false;
    
    // Build packet
    ByteBuffer packet;
    packet << keyno;
    packet << cryptogramEnc;

    BinaryData resp;
    if (!Transceive(DF_INS_CHANGE_KEY, packet.Data(), resp))
        return false;
    else
        return true;
}

DesfireKey Desfire::CreateSessionKey(const BinaryData& RndA, const BinaryData& RndB, const DesfireKey& key)
{
    ByteBuffer buf;

    switch(key.Type)
    {
        case DF_KEY_DES:
            buf.Append(RndA.data(), 4);
            buf.Append(RndB.data(), 4);
            return CreateDesfireKeyDES(buf.Data());
        case DF_KEY_3DES:
            buf.Append(RndA.data(), 4);
            buf.Append(RndB.data(), 4);
            buf.Append(RndA.data()+4, 4);
            buf.Append(RndB.data()+4, 4);
            return CreateDesfireKey3DES(buf.Data());
        case DF_KEY_3K3DES:
            buf.Append(RndA.data(), 4);
            buf.Append(RndB.data(), 4);
            buf.Append(RndA.data()+6, 4);
            buf.Append(RndB.data()+6, 4);
            buf.Append(RndA.data()+12, 4);
            buf.Append(RndB.data()+12, 4);
            return CreateDesfireKey3K3DES(buf.Data());
        case DF_KEY_AES:
            buf.Append(RndA.data(), 4);
            buf.Append(RndB.data(), 4);
            buf.Append(RndA.data()+12, 4);
            buf.Append(RndB.data()+12, 4);
            return CreateDesfireKeyAES(buf.Data());
    }

    // Return DF_KEY_NONE
    return DesfireKey();
}
