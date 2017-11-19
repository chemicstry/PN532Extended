#ifndef __DESFIRE_KEY_H__
#define __DESFIRE_KEY_H__

enum DesfireKeyType_t
{
    DF_KEY_NONE,
    DF_KEY_DES,
    DF_KEY_3DES,
    DF_KEY_3K3DES,
    DF_KEY_AES
}; 

struct DesfireKey
{
    DesfireKey(const BinaryData& key = BinaryData(), const DesfireKeyType_t type = DF_KEY_NONE) : Key(key), Type(type)
    {
        // Enforce key length
        switch (type)
        {
            case DF_KEY_DES:
            case DF_KEY_3DES:
                Key.resize(8);
                break;
            case DF_KEY_AES:
                Key.resize(16);
                break;
            case DF_KEY_3K3DES:
                Key.resize(24);
                break;
        }
    }

    BinaryData Key;
    DesfireKeyType_t Type;
};

inline DesfireKey CreateDesfireKeyDES(const BinaryData& key)
{
    return DesfireKey(key, DF_KEY_DES);
}

inline DesfireKey CreateDesfireKey3DES(const BinaryData& key)
{
    return DesfireKey(key, DF_KEY_3DES);
}

inline DesfireKey CreateDesfireKey3K3DES(const BinaryData& key)
{
    return DesfireKey(key, DF_KEY_3K3DES);
}

inline DesfireKey CreateDesfireKeyAES(const BinaryData& key)
{
    return DesfireKey(key, DF_KEY_AES);
}

#endif