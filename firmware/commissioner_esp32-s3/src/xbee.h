#pragma once

#include <stdint.h>
#include <stdbool.h>

// Frame type IDs
#define FRAME_TX_REQUEST   0x10
#define FRAME_AT_CMD       0x08
#define FRAME_AT_RESP      0x88
#define FRAME_ZB_RX        0x90
#define FRAME_TX_STATUS    0x8B
#define FRAME_MODEM_STATUS 0x8A

// TX
void xbee_send_raw(uint32_t sh, uint32_t sl, uint8_t cmd);

// AT commands
int  xbee_send_at(const char cmd[2], uint8_t *resp_buf, uint8_t buf_len);
void xbee_ping(void);

// Init / RX
void xbee_init(void);
void xbee_task(void *arg);