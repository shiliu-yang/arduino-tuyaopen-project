#ifndef __HARDWARE_CONFIG_H__
#define __HARDWARE_CONFIG_H__

#include "Arduino.h"

// button pin
#define BTN1_PIN        (24)
#define BTN2_PIN        (32)
#define BTN3_PIN        (34)

// buzzer pin
#define BUZZER_PIN      (36)

// audio in pin
#define AUDIO_IN_PIN    (25)

// pixel matrix pin
#define DATA_PIN            (16)
#define MATRIX_HEIGHT       (8)
#define MATRIX_WIDTH        (8)
#define MATRIX_COUNT        (4)

#define MATRIX_TOTAL_HEIGHT (8)
#define MATRIX_TOTAL_WIDTH  (32)
#define PIXEL_NUMBER        (MATRIX_TOTAL_HEIGHT * MATRIX_TOTAL_WIDTH)

#endif // __HARDWARE_CONFIG_H__
