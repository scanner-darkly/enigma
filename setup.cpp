#include "MIDI.h"
#include "setup.h"
#include "src/utility/debug.h"

USBHost usbHost;

// for some reason creating these in setup doesn't work, 
// they have to be initialized here instead
// simply add as many as you need

USBHub usbHubs[] = {
    USBHub(usbHost), 
    USBHub(usbHost),
    USBHub(usbHost),
    USBHub(usbHost)
};

MonomeSerial serialDevices[] = {
    MonomeSerial(usbHost),
    MonomeSerial(usbHost),
    MonomeSerial(usbHost),
    MonomeSerial(usbHost),
    MonomeSerial(usbHost),
    MonomeSerial(usbHost)
};

MIDIDevice midiDevices[] = {
    MIDIDevice(usbHost),
    MIDIDevice(usbHost),
    MIDIDevice(usbHost),
    MIDIDevice(usbHost),
    MIDIDevice(usbHost),
    MIDIDevice(usbHost)
};

const int HUB_COUNT = sizeof(usbHubs) / sizeof(usbHubs[0]);
const int SERIAL_DEVICE_COUNT = sizeof(serialDevices) / sizeof(serialDevices[0]);
const int MIDI_DEVICE_COUNT = sizeof(midiDevices) / sizeof(midiDevices[0]);
const int USB_DRIVER_COUNT = HUB_COUNT + SERIAL_DEVICE_COUNT + MIDI_DEVICE_COUNT;

UsbDriver usbDrivers[USB_DRIVER_COUNT];

Button buttons[BUTTON_COUNT] { Button(BUTTON_PIN_1, BUTTON_DEBOUNCE), Button(BUTTON_PIN_2, BUTTON_DEBOUNCE) };
LED led(LED_PIN);
LED leds[] { LED(LED_PIN_1), LED(LED_PIN_2) };

// create the hardware MIDI-out port
MIDI_CREATE_DEFAULT_INSTANCE();

void _setup() {
    for (int i = 0; i < USB_DRIVER_COUNT; i++) {
        usbDrivers[i].driver = NULL;
        usbDrivers[i].active = false;
        usbDrivers[i].monome = false;
    }

    int di = 0;

    for (int i = 0; i < HUB_COUNT; i++) {
        usbDrivers[di].driver = &usbHubs[i];
        usbDrivers[di++].name = "Hub" + i;
    }
    
    for (int i = 0; i < SERIAL_DEVICE_COUNT; i++) {
        usbDrivers[di].driver = &serialDevices[i];
        usbDrivers[di++].name = "Serial" + i;
    }

    for (int i = 0; i < MIDI_DEVICE_COUNT; i++) {
        usbDrivers[di].driver = &midiDevices[i];
        usbDrivers[di++].name = "MIDI" + i;
    }

    while (!Serial && (millis() < 5000));
    Serial.begin(115200);

    // Wait 1.5 seconds before turning on USB Host. If connected USB devices
    // use too much power, Teensy at least completes USB enumeration, which
    // makes isolating the power issue easier.
    if (millis() < 1500) delay(2000 - millis());
    usbHost.begin();

    MIDI.begin(MIDI_CHANNEL_OMNI);
    debugln(INFO, "--- setup done");
}
