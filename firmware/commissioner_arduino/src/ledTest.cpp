#include "ledTest.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

// Send single byte payload to target XBee address (on = 0x01, off = 0x00)
// 0xFFFE forces 64-bit address routing, avoids stale 16-bit address issues
// void sendLED(XBee& xbee, XBeeAddress64 addr, uint8_t state) {
//     uint8_t payload[] = { state };
//     ZBTxRequest tx = ZBTxRequest(addr, 0xFFFE, 0, 0, payload, sizeof(payload), 0);
//     xbee.send(tx);
// }

void sendLED(XBee& xbee, XBeeAddress64 addr, uint8_t state) {
    // D1 high = 0x05, D1 low = 0x04
    uint8_t d1Cmd[] = {'D', '1'};
    uint8_t cmdValue[] = { state ? 0x05 : 0x04 }; // 0x05 = Digital High, 0x04 = Digital Low
    
    RemoteAtCommandRequest remoteAtRequest = RemoteAtCommandRequest(
        addr,           // 64-bit destination address
        d1Cmd,          // AT command: D1
        cmdValue,       // value: high or low
        sizeof(cmdValue)
    );
    xbee.send(remoteAtRequest);
}

void LEDon(XBee& xbee) {
    sendLED(xbee, XBeeAddress64(JOINER0_SH, JOINER0_SL), 0x01); // Send on PL data to Joiner 0
    sendLED(xbee, XBeeAddress64(JOINER1_SH, JOINER1_SL), 0x01); // Send on PL data to Joiner 1
}

void LEDoff(XBee& xbee) {
    sendLED(xbee, XBeeAddress64(JOINER0_SH, JOINER0_SL), 0x00); // Send off PL data to Joiner 0
    sendLED(xbee, XBeeAddress64(JOINER1_SH, JOINER1_SL), 0x00); // Send off PL data to Joiner 1
}

void ledCmd(XBee& xbee, String nodeId, uint8_t state) {
    if (nodeId == "SS_0") {
        sendLED(xbee, XBeeAddress64(JOINER0_SH, JOINER0_SL), state);
        Serial.println("SS_0 LED -> " + String(state ? "ON" : "OFF"));
    } else if (nodeId == "SS_1") {
        sendLED(xbee, XBeeAddress64(JOINER1_SH, JOINER1_SL), state);
        Serial.println("SS_1 LED -> " + String(state ? "ON" : "OFF"));
    } else if (nodeId == "ALL") {
        sendLED(xbee, XBeeAddress64(JOINER0_SH, JOINER0_SL), state);
        sendLED(xbee, XBeeAddress64(JOINER1_SH, JOINER1_SL), state);
        Serial.println("ALL LED -> " + String(state ? "ON" : "OFF"));
    } else {
        Serial.println("ERROR: unknown node");
    }
    Serial.println();
}