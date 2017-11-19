#ifndef __DESFIRE_H__
#define __DESFIRE_H__

#include "CardInterface.h"
#include "ByteBuffer.h"
#include "DesfireKey.h"

enum ISO7816_4_CLA_t : uint8_t
{
    ISO7816_4_CLA_WITHOUT_SM_LAST   = 0x00, // CLA without SM (last or only command of a chain)
    ISO7816_4_CLA_WITH_SM_LAST      = 0x0C, // CLA with SM and header authentication (last or only command of a chain)
    ISO7816_4_CLA_WITHOUT_SM_CONT   = 0x10, // CLA without SM (command is not the last command of a chain)
    ISO7816_4_CLA_WITH_SM_CONT      = 0x1C  // CLA with SM and header authentication (command is not the last command of a chain)
};

struct ISO7816_4_CAPDU
{
    uint8_t CLA;        // Command class
    uint8_t INS;        // Instruction
    uint8_t P1;         // Parameter 1
    uint8_t P2;         // Parameter 2
    uint8_t Lc;         // Data length
    BinaryData Data;    // Data
    uint8_t Le;         // Something that is always zero
};

ByteBuffer& operator<<(ByteBuffer& a, const ISO7816_4_CAPDU& b);

struct ISO7816_4_RAPDU
{
    BinaryData Data;
    uint8_t SW1;        // 0x90 - command correct
    uint8_t SW2;        // DesfireStatus_t
};

ByteBuffer& operator>>(ByteBuffer& a, ISO7816_4_RAPDU& b);

// Unique desfire AID used in select file ISO7816-4 instruction
#define DESFIRE_AID {0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x00}

enum DesfireInstruction_t : uint8_t
{
    // Legacy instructions
    DF_INS_AUTHENTICATE_LEGACY        = 0x0A,
    DF_INS_CHANGE_KEY_SETTINGS        = 0x54,
    DF_INS_GET_KEY_SETTINGS           = 0x45,
    DF_INS_CHANGE_KEY                 = 0xC4,
    DF_INS_GET_KEY_VERSION            = 0x64,
    DF_INS_CREATE_APPLICATION         = 0xCA,
    DF_INS_DELETE_APPLICATION         = 0xDA,
    DF_INS_GET_APPLICATION_IDS        = 0x6A,
    DF_INS_SELECT_APPLICATION         = 0x5A,
    DF_INS_FORMAT_PICC                = 0xFC,
    DF_INS_GET_VERSION                = 0x60,
    DF_INS_GET_FILE_IDS               = 0x6F,
    DF_INS_GET_FILE_SETTINGS          = 0xF5,
    DF_INS_CHANGE_FILE_SETTINGS       = 0x5F,
    DF_INS_CREATE_STD_DATA_FILE       = 0xCD,
    DF_INS_CREATE_BACKUP_DATA_FILE    = 0xCB,
    DF_INS_CREATE_VALUE_FILE          = 0xCC,
    DF_INS_CREATE_LINEAR_RECORD_FILE  = 0xC1,
    DF_INS_CREATE_CYCLIC_RECORD_FILE  = 0xC0,
    DF_INS_DELETE_FILE                = 0xDF,
    DF_INS_READ_DATA                  = 0xBD,
    DF_INS_WRITE_DATA                 = 0x3D,
    DF_INS_GET_VALUE                  = 0x6C,
    DF_INS_CREDIT                     = 0x0C,
    DF_INS_DEBIT                      = 0xDC,
    DF_INS_LIMITED_CREDIT             = 0x1C,
    DF_INS_WRITE_RECORD               = 0x3B,
    DF_INS_READ_RECORDS               = 0xBB,
    DF_INS_CLEAR_RECORD_FILE          = 0xEB,
    DF_COMMIT_TRANSACTION             = 0xC7,
    DF_INS_ABORT_TRANSACTION          = 0xA7,
    DF_INS_ADDITIONAL_FRAME           = 0xAF,
    // Desfire EV1 instructions
    DFEV1_INS_AUTHENTICATE_ISO        = 0x1A,
    DFEV1_INS_AUTHENTICATE_AES        = 0xAA,
    DFEV1_INS_FREE_MEM                = 0x6E,
    DFEV1_INS_GET_DF_NAMES            = 0x6D,
    DFEV1_INS_GET_CARD_UID            = 0x51,
    DFEV1_INS_GET_ISO_FILE_IDS        = 0x61,
    DFEV1_INS_SET_CONFIGURATION       = 0x5C,
    DF_INS_MAX                        = 0xFF
};

enum DesfireStatus_t : uint8_t
{
    DF_STATUS_OPERATION_OK              = 0x00,
    DF_STATUS_NO_CHANGES                = 0x0C,
    DF_STATUS_OUT_OF_EEPROM_ERROR       = 0x0E,
    DF_STATUS_ILLEGAL_COMMAND_CODE      = 0x1C,
    DF_STATUS_INTEGRITY_ERROR           = 0x1E,
    DF_STATUS_NO_SUCH_KEY               = 0x40,
    DF_STATUS_LENGTH_ERROR              = 0x7E,
    DF_STATUS_PERMISSION_ERROR          = 0x9D,
    DF_STATUS_PARAMETER_ERROR           = 0x9E,
    DF_STATUS_APPLICATION_NOT_FOUND     = 0xA0,
    DF_STATUS_APPL_INTEGRITY_ERROR      = 0xA1,
    DF_STATUS_AUTHENTICATION_ERROR      = 0xAE,
    DF_STATUS_ADDITIONAL_FRAME          = 0xAF,
    DF_STATUS_BOUNDARY_ERROR            = 0xBE,
    DF_STATUS_PICC_INTEGRITY_ERROR      = 0xC1,
    DF_STATUS_COMMAND_ABORTED           = 0xCA,
    DF_STATUS_PICC_DISABLED_ERROR       = 0xCD,
    DF_STATUS_COUNT_ERROR               = 0xCE,
    DF_STATUS_DUPLICATE_ERROR           = 0xDE,
    DF_STATUS_EEPROM_ERROR              = 0xFE,
    DF_STATUS_FILE_NOT_FOUND            = 0xF0,
    DF_STATUS_FILE_INTEGRITY_ERROR      = 0xF1
};

inline void desfire_crc32_byte(uint32_t *crc, const uint8_t value)
{
    /* x32 + x26 + x23 + x22 + x16 + x12 + x11 + x10 + x8 + x7 + x5 + x4 + x2 + x + 1 */
    const uint32_t poly = 0xEDB88320;

    *crc ^= value;
    for (int current_bit = 7; current_bit >= 0; current_bit--) {
        int bit_out = (*crc) & 0x00000001;
        *crc >>= 1;
        if (bit_out)
            *crc ^= poly;
    }
}

class Desfire
{
public:
    Desfire(CardInterface& interface);

    bool Connect();
    bool Transceive(const DesfireInstruction_t ins, const BinaryData& in, BinaryData& out);

    DesfireInstruction_t GetAuthCmd(const DesfireKeyType_t& type);
    bool Authenticate(const uint8_t keyno, const DesfireKey& key);

    bool ChangeKey(uint8_t keyno, const DesfireKey& key);

    static DesfireKey CreateSessionKey(const BinaryData& RndA, const BinaryData& RndB, const DesfireKey& key);

    DesfireStatus_t GetLastError() const
    {
        return _lastError;
    }

private:
    uint8_t _selectedApplication;
    int8_t _authenticatedKeyNo;
    DesfireKey _sessionKey; // Gets assigned after successful authentication
    BinaryData _sessionKeyIV;
    DesfireStatus_t _lastError;
    CardInterface& _interface;
};

#endif
