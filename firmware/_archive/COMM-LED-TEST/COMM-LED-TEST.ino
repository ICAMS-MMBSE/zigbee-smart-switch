// Commissioner
// Monitors a push button and broadcasts LED state to joiners
// Setup: Push button connected to GND and GPIO 8 (arbitrary pin #, can be changed)

#include <XBee.h>
#include <SoftwareSerial.h>

SoftwareSerial xbeeSerial(2, 3);
XBee xbee = XBee();

// 64-bit Zigbee addresses of each joiner (SH + SL read from XCTU)
XBeeAddress64 joiner0 = XBeeAddress64(0x0013A200, 0x427638B1);
XBeeAddress64 joiner1 = XBeeAddress64(0x0013A200, 0x426E0F29);

#define LED_PIN 13  // Onboard LED
#define BTN_PIN 8   // Push button (active low)

// Tracks previous button state to detect changes
bool lastBtnState = HIGH;

// Send single byte payload to target XBee address (on = 0x01, off = 0x00)
void sendLED(XBeeAddress64 addr, uint8_t state)
{
    xbeeSerial.begin(115200);
    xbee.begin(xbeeSerial);
    uint8_t payload[] = { state };
    ZBTxRequest tx = ZBTxRequest(addr, payload, sizeof(payload));
    xbee.send(tx);
}

void LEDoff()
{
    digitalWrite(LED_PIN, LOW); // Turn off Commissioner LED
    sendLED(joiner0, 0x00);     // Send off PL data to Joiner 0
    sendLED(joiner1, 0x00);     // Send off PL data to Joiner 1
}

void LEDon()
{
    digitalWrite(LED_PIN, HIGH); // Turn on Commissioner LED
    sendLED(joiner0, 0x01);      // Send on PL data to Joiner 0
    sendLED(joiner1, 0x01);      // Send on PL data to Joiner 1
}

void setup() 
{
    pinMode(LED_PIN, OUTPUT);       // LED GPIO as output
    pinMode(BTN_PIN, INPUT_PULLUP); // Internal pull-up keeps pin HIGH at rest
}

void loop() 
{
    bool btnState = digitalRead(BTN_PIN); // Read current button state

    if (btnState != lastBtnState) // Only act on state changes
    {
        delay(100); // Debounce: wait and re-read to filter noise
        btnState = digitalRead(BTN_PIN);

        if (btnState == LOW) // Button pressed (pulled to GND)
        {
            LEDon();
        } 
        else // Button not pressed
        {                        
            LEDoff();
        }

        lastBtnState = btnState; // Update state for next iteration
    }
}