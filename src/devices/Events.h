#ifndef EVENTS_H
#define EVENTS_H

#include <Arduino.h>

class GridEvent {
    public:
        uint8_t x;
        uint8_t y;
        uint8_t pressed;
};

class EncoderEvent {
    public:
        uint8_t encoder;
        int8_t delta;
};

#endif
