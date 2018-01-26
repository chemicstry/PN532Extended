#ifndef _PN532INTERFACE_H_
#define _PN532INTERFACE_H_

#include <stdint.h>

enum PN532FrameDirection
{
    PN532_FRAME_DIR_TO_PN532    = 0xD4,
    PN532_FRAME_DIR_TO_HOST     = 0xD5
};

enum PN532Error
{
    PN532_ERROR_INVALID_ACK     = -1,
    PN532_ERROR_TIMEOUT         = -2,
    PN532_ERROR_INVALID_FRAME   = -3,
    PN532_ERROR_NO_SPACE        = -4
};

#define PN532_ACK_WAIT_TIME 10 // ms

class PN532Interface
{
public:
    virtual void begin() = 0;
    virtual void wakeup() = 0;

    virtual int8_t writeCommand(const uint8_t *data, uint8_t len) = 0;
    virtual int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout = 1000) = 0;
};

#endif
