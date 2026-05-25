#include <XBee.h>
#include <SoftwareSerial.h>

/*This file uses AT commands to test the connection between the Xbee RCP
and the Arduino's ESP32-S3
*/

// SparkFun Shield: XBee is on pins 2(RX) and 3(TX)
SoftwareSerial xbeeSerial(2, 3); // RX, TX

XBee xbee = XBee();

uint8_t shCmd[]    = {'S','H'};
uint8_t slCmd[]    = {'S','L'};
uint8_t assocCmd[] = {'A','I'};

AtCommandRequest atRequest = AtCommandRequest(shCmd);
AtCommandResponse atResponse = AtCommandResponse();

void setup() {
  Serial.begin(115200);        
  xbeeSerial.begin(115200);    
  xbee.begin(xbeeSerial);    // point library at the XBee serial port

  delay(5000);               // let XBee initialize
  Serial.println("Starting AT command test...");
}

void loop() {
  atRequest.setCommand(shCmd);
  sendAtCommand();

  atRequest.setCommand(slCmd);
  sendAtCommand();

  atRequest.setCommand(assocCmd);
  sendAtCommand();

  while (1) {}; // stop after one pass
}

void sendAtCommand() {
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