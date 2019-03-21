#ifndef MONOMESERIAL_H
#define MONOMESERIAL_H

#include <Arduino.h>
#include "USBHost_t36.h"
#include "Events.h"
#include "EventQueue.h"

class MonomeSerial : public USBSerial, public EventQueue {
    public: 
        MonomeSerial(USBHost usbHost);
        void initialize();
        void setupAsGrid(uint8_t _rows, uint8_t _columns);
        void setupAsArc(uint8_t _encoders);
        void getDeviceInfo();
        void poll();
        void refresh();

        void setGridLed(uint8_t x, uint8_t y, uint8_t level);
        void clearGridLed(uint8_t x, uint8_t y);
        void setArcLed(uint8_t enc, uint8_t led, uint8_t level);
        void clearArcLed(uint8_t enc, uint8_t led);
        void clearArcRing(uint8_t ring);
        void clearAllLeds();
        void refreshGrid();
        void refreshArc();

        bool active;
        bool isMonome;
        bool isGrid;
        uint8_t rows;
        uint8_t columns;
        uint8_t encoders;
        
    private : 
        static const int MAX_LED_COUNT = 512;
        uint8_t leds[MAX_LED_COUNT];
        bool arcDirty;
        bool gridDirty;

        MonomeSerial();
        void processSerial();
};

#endif
