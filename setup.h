#ifndef SETUP_H
#define SETUP_H

#include <Arduino.h>
#include "USBHost_t36.h"
#include <Bounce.h>
#include "src/devices/MonomeSerial.h"

const int BUTTON_COUNT = 2;
const int BUTTON_DEBOUNCE = 10; // ms
const int BUTTON_PIN_1 = 7;
const int BUTTON_PIN_2 = 8;
const int PRESSED = 1;
const int RELEASED = 0;

const int LED_PIN = 13; // teensy built-in LED
const int LED_PIN_1 = 23; // enigma LED 1
const int LED_PIN_2 = 22; // enigma LED 2

const int USB_BAUD = 115200;
const int USB_FORMAT = USBHOST_SERIAL_8N1;

class UsbDriver {
    public:
        String name;
        bool active;
        bool monome;
        USBDriver *driver;
};

class Button : public Bounce {
    public:
        Button(int pin, int debounce) : Bounce(pin, debounce) {
            event = -1;
            pinMode(pin, INPUT_PULLUP);
        };
        
        void poll(void) { 
            update(); 
            if (fallingEdge())
                event = PRESSED;
            else if (risingEdge())
                event = RELEASED;
            else
                event = -1;
        }
        
        int event;
};

class LED {
    public:
        LED(int pin) { _pin = pin; _on = false; pinMode(pin, OUTPUT); };
        void on(void) { digitalWrite(_pin, HIGH); };
        void off(void) { digitalWrite(_pin, LOW); };
        void toggle(void) { _on = !_on; if (_on) on(); else off(); }
    
    private:
        LED();
        int _pin;
        int _on;
};

extern USBHost usbHost;
extern UsbDriver usbDrivers[];

extern USBHub usbHub[];
extern MonomeSerial serialDevices[];
extern MIDIDevice midiDevices[];

extern const int USB_DRIVER_COUNT;
extern const int SERIAL_DEVICE_COUNT;

extern Button buttons[];
extern LED led;
extern LED leds[];

void _setup(void);

#endif
