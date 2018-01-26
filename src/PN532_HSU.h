#ifndef _PN532HSU_H_
#define _PN532HSU_H_

#include "PN532Interface.h"
#include "Arduino.h"

#define PN532_HSU_READ_TIMEOUT  1000
#define PN532_HSU_SPEED         115200

class PN532_HSU : public PN532Interface {
public:
    PN532_HSU(HardwareSerial &serial);

    void begin();
    void wakeup();
    virtual int8_t writeCommand(const uint8_t *data, uint8_t len);
    int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout);
private:
    HardwareSerial* _serial;
    uint8_t command;

    int8_t readAckFrame();
    int16_t receive(uint8_t *buf, int len, uint16_t timeout=PN532_HSU_READ_TIMEOUT);
    void cleanReceiveBuffer();
};

#endif
