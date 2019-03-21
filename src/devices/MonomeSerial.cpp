#include "MonomeSerial.h"
#include "../utility/debug.h"

MonomeSerial::MonomeSerial(USBHost usbHost) : USBSerial(usbHost) {
    initialize();
}

void MonomeSerial::initialize() {
    active = false;
    isMonome = false;
    isGrid = true;
    rows = 0;
    columns = 0;
    encoders = 0;
    clearQueue();
    clearAllLeds();
    arcDirty = false;
    gridDirty = false;
}

void MonomeSerial::setupAsGrid(uint8_t _rows, uint8_t _columns) {
    initialize();
    active = true;
    isMonome = true;
    isGrid = true;
    rows = _rows;
    columns = _columns;
    gridDirty = true;
    debugfln(INFO, "GRID rows: %d columns %d", rows, columns);
}

void MonomeSerial::setupAsArc(uint8_t _encoders) {
    initialize();
    active = true;
    isMonome = true;
    isGrid = false;
    encoders = _encoders;
    arcDirty = true;
    debugfln(INFO, "ARC encoders: %d", encoders);
}

void MonomeSerial::getDeviceInfo() {
    debugln(INFO, "MonomeSerial::getDeviceInfo");
    write(uint8_t(0));
    /*
    write(1);
    poll();
    write(3);
    poll();
    write(5);
    poll();
    write(0x0F);
    poll();
    */
}

void MonomeSerial::poll() {
    while (isMonome && available()) { processSerial(); };
}

void MonomeSerial::refresh() {
    if (!active) return;
  
    uint8_t buf[35];
    int ind, led;

    if (isGrid && gridDirty) {
        buf[0] = 0x1A;
        buf[1] = 0;
        buf[2] = 0;

        ind = 3;
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x += 2) {
                led = (y << 4) + x;
                buf[ind++] = (leds[led] << 4) | leds[led + 1];
            }
        write(buf, 35);
        
        ind = 3;
        buf[1] = 8;
        for (int y = 0; y < 8; y++)
            for (int x = 8; x < 16; x += 2) {
                led = (y << 4) + x;
                buf[ind++] = (leds[led] << 4) | leds[led + 1];
            }
        write(buf, 35);
        
        ind = 3;
        buf[1] = 0;
        buf[2] = 8;
        for (int y = 8; y < 16; y++)
            for (int x = 0; x < 8; x += 2) {
                led = (y << 4) + x;
                buf[ind++] = (leds[led] << 4) | leds[led + 1];
            }
        write(buf, 35);

        ind = 3;
        buf[1] = 8;
        for (int y = 8; y < 16; y++)
            for (int x = 8; x < 16; x += 2) {
                led = (y << 4) + x;
                buf[ind++] = (leds[led] << 4) | leds[led + 1];
            }
        write(buf, 35);
        
        gridDirty = false;
    }

    if (!isGrid && arcDirty) {
        buf[0] = 0x92;

        buf[1] = 0;
        ind = 2;
        for (led = 0; led < 64; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        write(buf, 34);
        
        buf[1] = 1;
        ind = 2;
        for (led = 64; led < 128; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        write(buf, 34);

        buf[1] = 2;
        ind = 2;
        for (led = 128; led < 192; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        write(buf, 34);
        
        buf[1] = 3;
        ind = 2;
        for (led = 192; led < 256; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        write(buf, 34);
        
        buf[1] = 4;
        ind = 2;
        for (led = 256; led < 320; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        write(buf, 34);

        buf[1] = 5;
        ind = 2;
        for (led = 320; led < 384; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        write(buf, 34);

        buf[1] = 6;
        ind = 2;
        for (led = 384; led < 448; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        write(buf, 34);

        buf[1] = 7;
        ind = 2;
        for (led = 448; led < 512; led += 2)
            buf[ind++] = (leds[led] << 4) | leds[led + 1];
        write(buf, 34);

        arcDirty = 0;
    }
}

void MonomeSerial::setGridLed(uint8_t x, uint8_t y, uint8_t level) {
    int index = x + (y << 4);
    if (index < MAX_LED_COUNT) leds[index] = level;
}

void MonomeSerial::clearGridLed(uint8_t x, uint8_t y) {
    setGridLed(x, y, 0);
}

void MonomeSerial::setArcLed(uint8_t enc, uint8_t led, uint8_t level) {
    int index = led + (enc << 6);
    if (index < MAX_LED_COUNT) leds[index] = level;
}
        
void MonomeSerial::clearArcLed(uint8_t enc, uint8_t led) {
    setArcLed(enc, led, 0);
}

void MonomeSerial::clearArcRing(uint8_t ring) {
    for (int i = ring << 6, upper = i + 64; i < upper; i++) leds[i] = 0;
}

void MonomeSerial::clearAllLeds() {
    for (int i = 0; i < MAX_LED_COUNT; i++) leds[i] = 0;
}

void MonomeSerial::refreshGrid() {
    gridDirty = true;
}

void MonomeSerial::refreshArc() {
    arcDirty = true;
}

void MonomeSerial::processSerial() {
    uint8_t devSect, devNum, readX, readY, gridNum, index;
    int8_t delta;
    String devID;

    // get command identifier: first byte of packet is identifier in the form: [(a << 4) + b]
    // a = section (ie. system, key-grid, digital, encoder, led grid, tilt)
    // b = command (ie. query, enable, led, key, frame)
    uint8_t identifierSent = read();  

    switch (identifierSent) {
        case 0x00: // device information
            // [null, "led-grid", "key-grid", "digital-out", "digital-in", "encoder", "analog-in", "analog-out", "tilt", "led-ring"]
            devSect = read();
            devNum = read();
            debugfln(INFO, "MonomeSerial::processSerial 0x00 system / query, section: %d number: %d", devSect, devNum);
            if (devSect == 2)
                setupAsGrid(devNum > 2 ? 16 : 8, devNum > 1 ? 16 : 8);
            else if (devSect == 5)
                setupAsArc(devNum);
            break;

        case 0x01: // system / ID
            debug(INFO, "MonomeSerial::processSerial 0x01 system / ID '");
            // for (int i = 0; i < 32; i++) { // has to be 32?
            for (int i = 0; i < 32 && available(); i++) {
                devID = read();
                debug(INFO, devID);
            }
            debugln(INFO, "'");
            break;

        case 0x02: // system / report grid offset - 4 bytes
            gridNum = read();  // n = grid number
            readX = read(); // an offset of 8 is valid only for 16 x 8 monome
            readY = read();  // an offset is invalid for y as it's only 8
            debugfln(INFO, "MonomeSerial::processSerial 0x02 system / report grid offset, n: %d x: %d y: %d", gridNum, readX, readY);
            break;

        case 0x03: // system / report grid size
            readX = read(); // an offset of 8 is valid only for 16 x 8 monome
            readY = read(); // an offset is invalid for y as it's only 8
            debugfln(INFO, "MonomeSerial::processSerial 0x03 system / grid offsets, x: %d y: %d", readX, readY);
            break;

        case 0x04: // system / report ADDR
            Serial.println("0x04");
            readX = read(); // a ADDR
            readY = read(); // b type
            debugfln(INFO, "MonomeSerial::processSerial 0x04, ADDR: %d type: %d", readX, readY);
            break;

        case 0x05: // system / report ADDR
            readX = read();
            readY = read();
            debugfln(INFO, "MonomeSerial::processSerial 0x05 system / grid size, x size: %d y size: %d", readX, readY);

        case 0x0F: // system / report firmware version
            debug(INFO, "MonomeSerial::processSerial 0x0F system firmware version ");
            // 8 character string
            for (int i = 0; i < 8; i++) debugf(INFO, "%d", read());
            debugln(INFO);
            break;

        case 0x20: // key-grid / key up
            // bytes: 3 structure: [0x20, x, y]  description: key up at (x,y)
            readX = read();
            readY = read();
            addGridEvent(readX, readY, 0);
            debugfln(INFO, "Grid key x: %d y: %d up", readX, readY);
            break;
            
        case 0x21: // key-grid / key down
            readX = read();
            readY = read();
            addGridEvent(readX, readY, 1);
            debugfln(INFO, "Grid key x: %d y: %d down", readX, readY);
            break;

        case 0x40: // d-line-in / change to low
            break;

        case 0x41: // d-line-in / change to high
            break;

        case 0x50: // arc encoder position change
            // bytes: 3 structure: [0x50, n, d] n = encoder number 0..255 d = delta -128..127
            index = read();
            delta = read();
            addEncoderEvent(index, delta);
            debugfln(INFO, "Arc encoder index: %d delta: %d", index, delta);
            break;

        case 0x51: // arc key up
            index = read();
            debugfln(INFO, "Arc key index: %d up", index);
            break;

        case 0x52: // arc key down
            index = read();
            debugfln(INFO, "Arc key index: %d down", index);
            break;

        case 0x60: // analog / active response - 33 bytes [0x01, d0..31]
            break;

        case 0x61: // analog in - 4 bytes [0x61, n, dh, dl]
            break;

        case 0x80: // tilt / active response - 9 bytes [0x01, d]
            break;

        case 0x81: // tilt - 8 bytes [0x80, n, xh, xl, yh, yl, zh, zl]
            break;

        default: 
            break;
    }
}
