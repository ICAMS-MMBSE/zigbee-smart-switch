#include "ledTest.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

void sendLED(XBee& xbee, XBeeAddress64 addr, uint8_t state) {
    uint8_t payload[] = { state };
    ZBTxRequest tx = ZBTxRequest(addr, payload, sizeof(payload));
    xbee.send(tx);
}

void LEDon(XBee& xbee) {
    sendLED(xbee, XBeeAddress64(JOINER0_SH, JOINER0_SL), 0x01);
    sendLED(xbee, XBeeAddress64(JOINER1_SH, JOINER1_SL), 0x01);
}

void LEDoff(XBee& xbee) {
    sendLED(xbee, XBeeAddress64(JOINER0_SH, JOINER0_SL), 0x00);
    sendLED(xbee, XBeeAddress64(JOINER1_SH, JOINER1_SL), 0x00);
}