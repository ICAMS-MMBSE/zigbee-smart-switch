#include <Arduino.h>
#include <XBee.h>
#include <SoftwareSerial.h>

SoftwareSerial xbeeSerial(2, 3);
XBee xbee = XBee();
ZBRxResponse rx = ZBRxResponse();

#define LED_PIN 13

void setup() {
    xbeeSerial.begin(115200);
    xbee.begin(xbeeSerial);
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {
        if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
            xbee.getResponse().getZBRxResponse(rx);
            uint8_t cmd = rx.getData(0);
            digitalWrite(LED_PIN, cmd ? HIGH : LOW);
        }
    }
}