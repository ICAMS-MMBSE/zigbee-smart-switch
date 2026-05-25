#pragma once
#include <XBee.h>

// Queries SH, SL, and AI from the local XBee via AT commands
// Prints results over Serial, ends with blank line for Python CLI
void atPing(XBee& xbee);