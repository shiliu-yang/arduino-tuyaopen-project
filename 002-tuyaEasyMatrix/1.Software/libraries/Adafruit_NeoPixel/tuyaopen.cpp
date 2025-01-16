#ifndef __TUYAOPEN_H__
#define __TUYAOPEN_H__

#include <Arduino.h>

#if defined(ARDUINO_T2) || defined(ARDUINO_T3)

#include "SPI.h"
#include "tal_log.h"
#include "tal_memory.h"

#define SPI_CLOCK   (3000000UL) // 400Khz

static uint8_t is_init = 0;

char *send_data = NULL;
size_t send_len_max = 0;

void tuyaopenShow(uint8_t pin, uint8_t *pixels, uint32_t numBytes, boolean is800KHz)
{
    if (!is_init) {
        SPI.begin();
        SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
        is_init = 1;
    }

    if (numBytes == 0 || pixels == NULL) {
        return;
    }

    if (NULL == send_data) {
        send_len_max = numBytes*8/2;
        send_data = (char *)tal_malloc(numBytes*8/2);
        if (send_data == NULL) {
            PR_ERR("malloc failed");
            return;
        }
    }

    size_t send_len = 0;
    memset(send_data, 0, send_len_max);

    uint8_t *p = (uint8_t *)send_data;
    uint8_t tmp_data = 0;
    for (uint32_t i = 0; i < numBytes; i++) {
        uint8_t data = pixels[i];
        for (uint8_t j = 0; j < 8; j+=2) {
            tmp_data = 0;
            if (data & 0x80) {
                // 1100
                tmp_data = 0xC0;
            } else {
                // 1000
                tmp_data = 0x80;
            }
            data <<= 1;
            if (data & 0x80) {
                // 1100
                tmp_data |= 0x0C;
            } else {
                // 1000
                tmp_data |= 0x08;
            }
            data <<= 1;
            *p++ = tmp_data;
            send_len++;
            if (send_len >= send_len_max) {
                break;
            }
        }
    }

    SPI.transfer((void *)send_data, send_len);
    // SPI.endTransaction();
    send_len = 0;

    // tal_free(send_data);
    // send_len_max = 0;

    return;
}

#endif // ARDUINO_T2 || ARDUINO_T3

#endif // __TUYAOPEN_H__