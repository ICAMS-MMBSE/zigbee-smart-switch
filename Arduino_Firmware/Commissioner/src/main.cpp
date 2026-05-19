#include <Arduino.h>
#include <XBee.h>
#include <SoftwareSerial.h>
#include "ledTest.h"

SoftwareSerial xbeeSerial(2, 3);
XBee xbee = XBee();

#define LED_PIN 13
#define BTN_PIN 8

bool lastBtnState = HIGH;

void setup() {
    xbeeSerial.begin(115200);
    xbee.begin(xbeeSerial);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
}

void loop() {
    bool btnState = digitalRead(BTN_PIN);
    if (btnState != lastBtnState) {
        delay(100);
        btnState = digitalRead(BTN_PIN);
        if (btnState == LOW) {
            LEDon(xbee);
            digitalWrite(LED_PIN, HIGH);
        } else {
            LEDoff(xbee);
            digitalWrite(LED_PIN, LOW);
        }
        lastBtnState = btnState;
    }
}