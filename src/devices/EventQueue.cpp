#include "EventQueue.h"
#include "../utility/debug.h"

EventQueue::EventQueue() {
    clearQueue();
}

void EventQueue::clearQueue() {
    gridEventCount = gridFirstEvent = 0;
    encoderEventCount = encoderFirstEvent = 0;
}

void EventQueue::addGridEvent(uint8_t x, uint8_t y, uint8_t pressed) {
    if (gridEventCount >= MAX_EVENT_COUNT) {
        debugln(WARN, "Grid event queue full!");
        return;
    }
    uint8_t ind = (gridFirstEvent + gridEventCount) % MAX_EVENT_COUNT;
    gridEvents[ind].x = x;
    gridEvents[ind].y = y;
    gridEvents[ind].pressed = pressed;
    gridEventCount++;
}

bool EventQueue::gridEventAvailable() {
    return gridEventCount > 0;
}

GridEvent EventQueue::readGridEvent() {
    if (gridEventCount == 0) {
        debugln(WARN, "Reading from an empty grid event queue!");
        return emptyGridEvent;
    }
    gridEventCount--;
    uint8_t index = gridFirstEvent;
    gridFirstEvent = (gridFirstEvent + 1) % MAX_EVENT_COUNT;
    return gridEvents[index];
}

void EventQueue::addEncoderEvent(uint8_t encoder, int8_t delta) {
    if (encoderEventCount >= MAX_EVENT_COUNT) {
        debugln(WARN, "Encoder event queue full!");
        return;
    }
    uint8_t ind = (encoderFirstEvent + encoderEventCount) % MAX_EVENT_COUNT;
    encoderEvents[ind].encoder = encoder;
    encoderEvents[ind].delta = delta;
    encoderEventCount++;
}

bool EventQueue::encoderEventAvailable() {
    return encoderEventCount > 0;
}

EncoderEvent EventQueue::readEncoderEvent() {
    if (encoderEventCount == 0) {
        debugln(WARN, "Reading from an empty encoder event queue!");
        return emptyEncoderEvent;
    }
    encoderEventCount--;
    uint8_t index = encoderFirstEvent;
    encoderFirstEvent = (encoderFirstEvent + 1) % MAX_EVENT_COUNT;
    return encoderEvents[index];
}
