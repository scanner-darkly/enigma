#include "loop.h"
#include "setup.h"
#include "src/utility/debug.h"

elapsedMillis blink;

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
        if (String(maker) == "monome") initMonomeDevice(i, pss);
    }
}

void _loop() {
    _updateDeviceInfo();
    usbHost.Task();

    for (int i = 0; i < SERIAL_DEVICE_COUNT; i++) 
        if (serialDevices[i].isMonome) serialDevices[i].poll();

    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttons[i].poll();
        if (buttons[i].event == PRESSED)
            leds[i].on();
        else if (buttons[i].event == RELEASED)
            leds[i].off();
    }

    if (blink > 500) {
        led.toggle();
        blink = 0;
    }
}
