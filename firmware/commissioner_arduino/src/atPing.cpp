// atPing
// Uses AT commands to test the connection between the XBee RCP and the Arduino

#include "atPing.h"
#include <Arduino.h>

// Helper: sends a single AT command and prints the response
static void sendAtCommand(XBee& xbee, AtCommandRequest& atRequest, AtCommandResponse& atResponse) {
    Serial.println("Sending command to XBee...");
    xbee.send(atRequest);
    if (xbee.readPacket(5000)) {
        if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
            xbee.getResponse().getAtCommandResponse(atResponse);
            if (atResponse.isOk()) {
                Serial.print("Command [");
                Serial.print((char)atResponse.getCommand()[0]);
                Serial.print((char)atResponse.getCommand()[1]);
                Serial.println("] OK!");
                Serial.print("Value: ");
                for (int i = 0; i < atResponse.getValueLength(); i++) {
                    Serial.print(atResponse.getValue()[i], HEX);
                    Serial.print(" ");
                }
                Serial.println();
            } else {
                Serial.print("Error code: ");
                Serial.println(atResponse.getStatus(), HEX);
            }
        }
    } else {
        if (xbee.getResponse().isError()) {
            Serial.print("Packet error code: ");
            Serial.println(xbee.getResponse().getErrorCode());
        } else {
            Serial.println("No response from XBee - check baud rate and AP=2");
        }
    }
}

void atPing(XBee& xbee) {
    uint8_t shCmd[]    = {'S', 'H'};
    uint8_t slCmd[]    = {'S', 'L'};
    uint8_t assocCmd[] = {'A', 'I'};

    AtCommandRequest atRequest = AtCommandRequest(shCmd);
    AtCommandResponse atResponse = AtCommandResponse();

    atRequest.setCommand(shCmd);
    sendAtCommand(xbee, atRequest, atResponse);

    atRequest.setCommand(slCmd);
    sendAtCommand(xbee, atRequest, atResponse);

    atRequest.setCommand(assocCmd);
    sendAtCommand(xbee, atRequest, atResponse);

    Serial.println(); // blank line signals end of response to Python CLI
}