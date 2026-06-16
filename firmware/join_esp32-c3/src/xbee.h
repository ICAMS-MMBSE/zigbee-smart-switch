#pragma once

#include <stdbool.h>
#include <stdint.h>

// XBee API frame types
#define FRAME_TX_REQUEST   0x10
#define FRAME_ZB_RX        0x90

void xbee_init(void);
void xbee_task(void *arg);
void xbee_report_state(bool state);
