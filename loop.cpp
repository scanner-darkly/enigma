#include "loop.h"
#include "setup.h"
#include "src/utility/debug.h"

elapsedMillis monomeRefreshTimer;

elapsedMillis blink;
const int MAX_ENCODER_COUNT = 8;
int arcValues[MAX_ENCODER_COUNT] = { 0 };

void initMonomeDevice(int driver, const char *pss) {
    debugln(INFO, "initMonomeDevice");

    int serialnum;
    usbDrivers[driver].monome = true;

    MonomeSerial *monome = (MonomeSerial *)usbDrivers[driver].driver;
    monome->begin(USB_BAUD, USB_FORMAT);
    monome->isMonome = true;

    // "m128%*1[-_]%d" = series, "mk%d" = kit, "m40h%d" = 40h, "m%d" = mext
    if (sscanf(pss, "m40h%d", &serialnum)) {
        debugln(INFO, "  40h device");
        monome->setupAsGrid(8, 8);
    }
    else if (sscanf(pss, "m256%*1[-_]%d", &serialnum)) {
        debugln(INFO, "  monome series 256 device");
        monome->setupAsGrid(16, 16);
    }
    else if (sscanf(pss, "m128%*1[-_]%d", &serialnum)) {
        debugln(INFO, "  monome series 128 device");
        monome->setupAsGrid(8, 16);
    }
    else if (sscanf(pss, "m64%*1[-_]%d", &serialnum)) {
        debugln(INFO, "  monome series 64 device");
        monome->setupAsGrid(8, 8);
    }
    else if (sscanf(pss, "mk%d", &serialnum)) {
        debugln(INFO, "   monome kit device");
        monome->setupAsGrid(8, 8);
    }
    else if (sscanf(pss, "m%d", &serialnum)) { 
        debugln(INFO, "  mext device");
        monome->getDeviceInfo();
        // mext devices will initiialize in getDeviceInfo()
    } else {
        debugfln(WARN, "unknown monome device: %s", pss);
    }
}

void _updateDeviceInfo() {
    char maker[8];

    for (int i = 0; i < USB_DRIVER_COUNT; i++) {
        if (*(usbDrivers[i].driver) == usbDrivers[i].active) continue;

        if (usbDrivers[i].active) {
            usbDrivers[i].active = false;
            if (usbDrivers[i].monome) ((MonomeSerial *)usbDrivers[i].driver)->initialize();
            debugfln(INFO, "*** %s Device - disconnected ***", usbDrivers[i].name.c_str());
            continue;
        }

        usbDrivers[i].active = true;
        
        // TODO if we need to support other serial devices we'll need to determine baud rate
        // and execute begin(). for monome it's done in initMonomeDevice

        debugfln(INFO, "*** %s Device %x:%x - connected ***", 
            usbDrivers[i].name.c_str(),
            usbDrivers[i].driver->idVendor(), usbDrivers[i].driver->idProduct());

        const uint8_t *psz = usbDrivers[i].driver->manufacturer();
        if (psz && *psz) debugfln(INFO, "  manufacturer: %s", psz);

        const char *psp = (const char *)usbDrivers[i].driver->product();
        if (psp && *psp) debugfln(INFO, "  product: %s", psp);

        const char *pss = (const char *)usbDrivers[i].driver->serialNumber();
        if (pss && *pss) debugfln(INFO, "  serial: %s", pss);

        sprintf(maker, "%s", psz);
        if (String(maker) == "monome" && usbDrivers[i].serial) initMonomeDevice(i, pss);
    }
}

void _loop() {
    _updateDeviceInfo();
    usbHost.Task();

    // blink teensy LED
    if (blink > 500) {
        led.toggle();
        blink = 0;
    }

    // light LEDs when buttons pressed
    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttons[i].poll();
        if (buttons[i].event == PRESSED)
            leds[i].on();
        else if (buttons[i].event == RELEASED)
            leds[i].off();
    }

    // simple grid/arc test
    for (int i = 0; i < SERIAL_DEVICE_COUNT; i++) {
        if (serialDevices[i].isMonome) serialDevices[i].poll();
        while (serialDevices[i].gridEventAvailable()) {
            GridEvent event = serialDevices[i].readGridEvent();
            serialDevices[i].setGridLed(event.x, event.y, event.pressed ? 15 : 0);
            serialDevices[i].refreshGrid();
        }
        while (serialDevices[i].encoderEventAvailable()) {
            EncoderEvent event = serialDevices[i].readEncoderEvent();
            if (event.encoder < MAX_ENCODER_COUNT) {
                arcValues[event.encoder] = (arcValues[event.encoder] + 64 + (event.delta > 0 ? 1 : -1)) & 63;
                debugln(INFO, arcValues[event.encoder]);
                serialDevices[i].clearArcRing(event.encoder);
                serialDevices[i].setArcLed(event.encoder, arcValues[event.encoder], 15);
                serialDevices[i].refreshArc();
            }
        }
    }

    if (monomeRefreshTimer > 50) {
        for (int i = 0; i < SERIAL_DEVICE_COUNT; i++) serialDevices[i].refresh();
        monomeRefreshTimer = 0;
    }
}
