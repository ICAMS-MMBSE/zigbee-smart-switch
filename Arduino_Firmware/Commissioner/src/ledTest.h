#pragma once
#include <XBee.h>

#define JOINER0_SH 0x0013A200
#define JOINER0_SL 0x426E1209

#define JOINER1_SH 0x0013A200
#define JOINER1_SL 0x426E0EB9

void sendLED(XBee& xbee, XBeeAddress64 addr, uint8_t state);
void LEDon(XBee& xbee);
void LEDoff(XBee& xbee);