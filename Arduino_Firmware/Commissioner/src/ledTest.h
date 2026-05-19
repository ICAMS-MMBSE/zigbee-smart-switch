#pragma once
#include <XBee.h>

// 64-bit Zigbee addresses of each joiner (SH + SL read from XCTU)
#define JOINER0_SH 0x0013A200
#define JOINER0_SL 0x426E1209

#define JOINER1_SH 0x0013A200
#define JOINER1_SL 0x426E0EB9

void sendLED(XBee& xbee, XBeeAddress64 addr, uint8_t state);
void LEDon(XBee& xbee);
void LEDoff(XBee& xbee);
void ledCmd(XBee& xbee, String nodeId, uint8_t state);