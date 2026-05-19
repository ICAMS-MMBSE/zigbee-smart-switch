// Commissioner
// Monitors a push button and broadcasts LED state to joiners
// Also listens for serial commands from the Python CLI (ZSS)
// Setup: Push button connected to GND and GPIO 8 (arbitrary pin #, can be changed)
#include <Arduino.h>
#include <XBee.h>
#include <SoftwareSerial.h>
#include "ledTest.h"
#include "atPing.h"

SoftwareSerial xbeeSerial(2, 3);
XBee xbee = XBee();

#define LED_PIN 13  // Onboard LED
#define BTN_PIN 8   // Push button (active low)

// Tracks previous button state to detect changes
bool lastBtnState = HIGH;

void setup() {
    Serial.begin(115200);           // USB serial for Python CLI communication
    xbeeSerial.begin(115200);       // UART to XBee module
    xbee.begin(xbeeSerial);         // Point XBee library at the software serial port
    pinMode(LED_PIN, OUTPUT);       // LED GPIO as output
    pinMode(BTN_PIN, INPUT_PULLUP); // Internal pull-up keeps pin HIGH at rest
}

void loop() {
    // Serial command parser -- receives commands from Python CLI
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();

        if (cmd == "AT_PING") {
            atPing(xbee);
        } else if (cmd.startsWith("LED ")) {
            // Format: LED <node_id> <0|1>  e.g. "LED SS_0 1"
            int lastSpace = cmd.lastIndexOf(' ');
            String nodeId = cmd.substring(4, lastSpace);
            uint8_t state = cmd.substring(lastSpace + 1).toInt();
            ledCmd(xbee, nodeId, state);
        } else {
            Serial.println("ERROR: unknown command");
            Serial.println(); // blank line signals end of response to Python
        }
    }

    // Button handler -- physical push button on commissioner
    bool btnState = digitalRead(BTN_PIN); // Read current button state
    if (btnState != lastBtnState) // Only act on state changes
    {
        delay(100); // Debounce: wait and re-read to filter noise
        btnState = digitalRead(BTN_PIN);
        if (btnState == LOW) // Button pressed (pulled to GND)
        {
            LEDon(xbee);                 // Send LEDon to joiners
            digitalWrite(LED_PIN, HIGH); // Turn on Commissioner LED
        } else // Button not pressed
        {
            LEDoff(xbee);                // Send LEDoff to joiners
            digitalWrite(LED_PIN, LOW);  // Turn off Commissioner LED
        }
        lastBtnState = btnState; // Update state for next iteration
    }
}