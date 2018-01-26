#include "PN532_HSU.h"

PN532_HSU::PN532_HSU(HardwareSerial &serial): _serial(&serial), command(0)
{
}

void PN532_HSU::begin()
{
    _serial->begin(PN532_HSU_SPEED);
}

void PN532_HSU::wakeup()
{
    // PN532 wake up condition
    _serial->write(0x55);
    _serial->write(0x55);
    _serial->write(0);
    _serial->write(0);
    _serial->write(0);

    // Consume response
    cleanReceiveBuffer();
}

int8_t PN532_HSU::writeCommand(const uint8_t *data, uint8_t len)
{
    // In case something is stuck
    cleanReceiveBuffer();

    // For checking response
    command = data[0];

    _serial->write(0x00); // Preamble
    _serial->write(0x00); // Start Code 0
    _serial->write(0xFF); // Start Code 1

    _serial->write(len+1); // LEN (Data length + TFI)
    _serial->write(~len);  // LCS (Checksum for LEN. Satisfies LOW_BYTE(LEN + LCS) = 0x00)

    _serial->write(PN532_FRAME_DIR_TO_PN532); // TFI

    uint8_t checksum = PN532_FRAME_DIR_TO_PN532;

    // Write data
    _serial->write(data, len);

    // calculate checksum
    for (uint8_t i = 0; i < len; i++)
        checksum += data[i];

    _serial->write(~checksum + 1); // DCS (TFI + data)
    _serial->write(0x00); // Postamble

    return readAckFrame();
}

int16_t PN532_HSU::readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint8_t tmp[3];

    // Read preamble and start code
    if (receive(tmp, 3, timeout) < 3)
        return PN532_ERROR_TIMEOUT;

    if (tmp[0] != 0x00 || tmp[1] != 0x00 || tmp[2] != 0xFF)
        return PN532_ERROR_INVALID_FRAME;

    // Receive LEN and LCS
    uint8_t length[2];
    if (receive(length, 2, timeout) < 2)
        return PN532_ERROR_TIMEOUT;

    if ((uint8_t)(length[0] + length[1]) != 0)
        return PN532_ERROR_INVALID_FRAME;

    length[0] -= 2; // Substract TFI and CMD

    // Check if buffer is big enough
    if (length[0] > len)
        return PN532_ERROR_NO_SPACE;

    // Receive TFI and CMD
    if (receive(tmp, 2, timeout) < 2)
        return PN532_ERROR_TIMEOUT;

    // Check TFI and CMD (response CMD is increased by 1)
    if (tmp[0] != PN532_FRAME_DIR_TO_HOST || tmp[1] != command + 1)
        return PN532_ERROR_INVALID_FRAME;

    // Receive data
    if (receive(buf, length[0], timeout) != length[0])
        return PN532_ERROR_TIMEOUT;

    // Calculate checksum
    uint8_t checksum = PN532_FRAME_DIR_TO_HOST + command + 1;
    for (uint8_t i=0; i < length[0]; i++)
        checksum += buf[i];

    // Receive DCS and postamble
    if (receive(tmp, 2, timeout) < 2)
        return PN532_ERROR_TIMEOUT;

    if ((uint8_t)(checksum + tmp[0]) || tmp[1] != 0x00)
        return PN532_ERROR_INVALID_FRAME;

    return length[0];
}

int8_t PN532_HSU::readAckFrame()
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
    uint8_t ackBuf[sizeof(PN532_ACK)];

    // Receive ACK
    if (receive(ackBuf, sizeof(PN532_ACK), PN532_ACK_WAIT_TIME) <= 0 )
        return PN532_ERROR_TIMEOUT;

    // Compare ACK
    if (memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK)))
        return PN532_ERROR_INVALID_ACK;

    return 0;
}

int16_t PN532_HSU::receive(uint8_t *buf, int len, uint16_t timeout)
{
    int i = 0;
    unsigned long start = millis();

    while (i < len)
    {
        int res = _serial->read();

        // Check if read is successful
        if (res >= 0)
            buf[i++] = (uint8_t)res;
        else
            delay(1); // Yield to kernel

        // Check if timeouted
        if (timeout && (millis() - start) > timeout)
        {
            if (i)
                return i;
            else
                return PN532_ERROR_TIMEOUT;
        }
    }

    return i;
}

void PN532_HSU::cleanReceiveBuffer()
{
    while (_serial->available())
        _serial->read();
}
