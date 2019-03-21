#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include "Arduino.h"
#include "Events.h"

class EventQueue {
    public:
        EventQueue();

        void clearQueue();

        bool gridEventAvailable();
        GridEvent readGridEvent();

        bool encoderEventAvailable();
        EncoderEvent readEncoderEvent();
        
    protected:
        void addGridEvent(uint8_t x, uint8_t y, uint8_t pressed);
        void addEncoderEvent(uint8_t encoder, int8_t delta);
        
    private:
        static const int MAX_EVENT_COUNT = 50;
        
        GridEvent emptyGridEvent;
        GridEvent gridEvents[MAX_EVENT_COUNT];
        int gridEventCount = 0;
        int gridFirstEvent = 0;

        EncoderEvent emptyEncoderEvent;
        EncoderEvent encoderEvents[MAX_EVENT_COUNT];
        int encoderEventCount = 0;
        int encoderFirstEvent = 0;
};

#endif
