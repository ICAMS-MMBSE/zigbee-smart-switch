// Joiner
// Listens for incoming Zigbee packets and controls LED

#include <Arduino.h>
#include <XBee.h>
#include <SoftwareSerial.h>

SoftwareSerial xbeeSerial(2, 3);
XBee xbee = XBee();
ZBRxResponse rx = ZBRxResponse();  // Reusable RX packet object

#define LED_PIN 13 // Onboard LED

void setup() {
    xbeeSerial.begin(115200);
    xbee.begin(xbeeSerial);
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    xbee.readPacket();  // Non-blocking check for incoming packet
    if (xbee.getResponse().isAvailable()) {
        // Check if the packet is a Zigbee RX frame (data from another node)
        if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
            xbee.getResponse().getZBRxResponse(rx);
            uint8_t cmd = rx.getData(0);  // First byte of payload is the command
            digitalWrite(LED_PIN, cmd ? HIGH : LOW);
        }
    }
}