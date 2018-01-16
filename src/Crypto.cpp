#include "Crypto.h"
#include <hwcrypto/aes.h>

BinaryData AES_CBC_Decrypt(const BinaryData& data, const BinaryData& key, BinaryData& iv)
{
    esp_aes_context ctx;
    esp_aes_init(&ctx);
    esp_aes_setkey(&ctx, key.data(), key.size()*8);

    // Output size is the same as input size
    BinaryData out(data.size());

    esp_aes_crypt_cbc(
        &ctx,
        ESP_AES_DECRYPT,
        data.size(),
        iv.data(),
        data.data(),
        out.data()
    );

    esp_aes_free(&ctx);

    return out;
}

BinaryData AES_CBC_Encrypt(const BinaryData& data, const BinaryData& key, BinaryData& iv)
{
    esp_aes_context ctx;
    esp_aes_init(&ctx);
    esp_aes_setkey(&ctx, key.data(), key.size()*8);

    // Output size is the same as input size
    BinaryData out(data.size());

    esp_aes_crypt_cbc(
        &ctx,
        ESP_AES_ENCRYPT,
        data.size(),
        iv.data(),
        data.data(),
        out.data()
    );

    esp_aes_free(&ctx);

    return out;
}