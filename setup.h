#ifndef SETUP_H
#define SETUP_H

#include <Arduino.h>
#include "USBHost_t36.h"
#include "src/devices/MonomeSerial.h"

const int USB_BAUD = 115200;
const int USB_FORMAT = USBHOST_SERIAL_8N1;

class UsbDriver {
    public:
        String name;
        bool active;
        bool monome;
        USBDriver *driver;
};

extern USBHost usbHost;
extern UsbDriver usbDrivers[];

extern USBHub usbHub[];
extern MonomeSerial serialDevices[];
extern MIDIDevice midiDevices[];

extern const int USB_DRIVER_COUNT;
extern const int SERIAL_DEVICE_COUNT;

void _setup(void);

#endif