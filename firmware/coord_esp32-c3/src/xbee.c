#include "xbee.h"
#include "mqtt.h"
#include "discovery.h"
#include "relay.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "XBEE";

// RX parser state (persists across parse_rx calls)
static uint8_t  rx_buf[128];
static uint8_t  rx_idx       = 0;
static uint16_t rx_frame_len = 0;
static bool     rx_in_frame  = false;

static uint8_t calc_checksum(uint8_t *frame, uint8_t len)
{
    uint8_t sum = 0;
    for (uint8_t i = 3; i < len - 1; i++) sum += frame[i];
    return 0xFF - sum;
}

static bool verify_checksum(uint8_t *buf, uint8_t len)
{
    return calc_checksum(buf, len) == buf[len - 1];
}

void xbee_init(void)
{
    uart_config_t cfg = {
        .baud_rate  = XBEE_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_driver_install(XBEE_UART, UART_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(XBEE_UART, &cfg);
    uart_set_pin(XBEE_UART, XBEE_TX, XBEE_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI(TAG, "UART init done");
}

// Build and send a ZBTxRequest (0x10) to a specific 64-bit address
void xbee_send_raw(uint32_t sh, uint32_t sl, uint8_t cmd)
{
    uint8_t frame[19];

    frame[0]  = 0x7E;
    frame[1]  = 0x00;
    frame[2]  = 0x0F;
    frame[3]  = FRAME_TX_REQUEST;
    frame[4]  = 0x01;
    frame[5]  = (sh >> 24) & 0xFF;
    frame[6]  = (sh >> 16) & 0xFF;
    frame[7]  = (sh >>  8) & 0xFF;
    frame[8]  =  sh        & 0xFF;
    frame[9]  = (sl >> 24) & 0xFF;
    frame[10] = (sl >> 16) & 0xFF;
    frame[11] = (sl >>  8) & 0xFF;
    frame[12] =  sl        & 0xFF;
    frame[13] = 0xFF;   // 16-bit network address (unknown)
    frame[14] = 0xFE;
    frame[15] = 0x00;   // broadcast radius
    frame[16] = 0x00;   // options
    frame[17] = cmd;
    frame[18] = calc_checksum(frame, 19);

    uart_write_bytes(XBEE_UART, (const char *)frame, 19);
}

// Send an AT command and block up to 2s waiting for the 0x88 response
int xbee_send_at(const char cmd[2], uint8_t *resp_buf, uint8_t buf_len)
{
    uint8_t frame[8];
    frame[0] = 0x7E;
    frame[1] = 0x00;
    frame[2] = 0x04;
    frame[3] = FRAME_AT_CMD;
    frame[4] = 0x01;
    frame[5] = cmd[0];
    frame[6] = cmd[1];
    frame[7] = calc_checksum(frame, 8);

    uart_write_bytes(XBEE_UART, (const char *)frame, 8);

    uint8_t  buf[64];
    uint8_t  idx       = 0;
    uint16_t frame_len = 0;
    bool     in_frame  = false;
    uint32_t start     = xTaskGetTickCount();

    while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(2000)) {
        uint8_t b;
        if (uart_read_bytes(XBEE_UART, &b, 1, pdMS_TO_TICKS(10)) != 1) continue;

        if (!in_frame) {
            if (b == 0x7E) {
                in_frame = true;
                idx = 0;
                buf[idx++] = b;
            }
        } else {
            buf[idx++] = b;
            if (idx == 3) {
                frame_len = ((uint16_t)buf[1] << 8) | buf[2];
            }
            if (frame_len > 0 && idx == (uint8_t)(frame_len + 4)) {
                if (!verify_checksum(buf, idx)) return -1;
                if (buf[3] != FRAME_AT_RESP)    return -1;
                if (buf[7] != 0x00)             return -1; // status: 0x00 = OK
                uint8_t val_len = frame_len - 5;
                if (val_len > buf_len) val_len = buf_len;
                memcpy(resp_buf, &buf[8], val_len);
                return val_len;
            }
            if (idx >= 64) return -1;
        }
    }
    return -1;
}

// Query SH, SL, and AI to verify the XBee is alive and on a network
void xbee_ping(void)
{
    uint8_t     resp[8];
    const char *cmds[] = {"SH", "SL", "AI"};

    for (uint8_t i = 0; i < 3; i++) {
        int len = xbee_send_at(cmds[i], resp, sizeof(resp));
        if (len < 0) {
            ESP_LOGW(TAG, "Command [%s] failed", cmds[i]);
            continue;
        }
        ESP_LOGI(TAG, "Command [%s] OK! Value:", cmds[i]);
        for (int j = 0; j < len; j++) {
            ESP_LOGI(TAG, "  0x%02X", resp[j]);
        }
    }
}

static void parse_rx(void)
{
    uint8_t b;
    while (uart_read_bytes(XBEE_UART, &b, 1, 0) == 1) {

        if (!rx_in_frame) {
            if (b == 0x7E) {
                rx_in_frame  = true;
                rx_idx       = 0;
                rx_frame_len = 0;
                rx_buf[rx_idx++] = b;
            }
        } else {
            rx_buf[rx_idx++] = b;

            if (rx_idx == 3) {
                rx_frame_len = ((uint16_t)rx_buf[1] << 8) | rx_buf[2];
            }

            if (rx_frame_len > 0 && rx_idx == (uint8_t)(rx_frame_len + 4)) {
                if (verify_checksum(rx_buf, rx_idx)) {
                    uint8_t frame_type = rx_buf[3];

                    if (frame_type == FRAME_ZB_RX) {
                        uint32_t src_sh = ((uint32_t)rx_buf[4]  << 24) |
                                          ((uint32_t)rx_buf[5]  << 16) |
                                          ((uint32_t)rx_buf[6]  <<  8) |
                                           rx_buf[7];
                        uint32_t src_sl = ((uint32_t)rx_buf[8]  << 24) |
                                          ((uint32_t)rx_buf[9]  << 16) |
                                          ((uint32_t)rx_buf[10] <<  8) |
                                           rx_buf[11];

                        // Payload byte 15 carries the relay state reported by the joiner
                        if (rx_frame_len > 12) {
                            uint8_t state = rx_buf[15];
                            for (uint8_t i = 0; i < nodeCount; i++) {
                                if (nodeTable[i].addr_sh == src_sh && nodeTable[i].addr_sl == src_sl) {
                                    ESP_LOGI(TAG, "%s state report: %s",
                                            nodeTable[i].node_id, state ? "ON" : "OFF");
                                    mqtt_publish_state(nodeTable[i].node_id, state);
                                    break;
                                }
                            }
                        }
                    }
                }
                rx_in_frame  = false;
                rx_idx       = 0;
                rx_frame_len = 0;
            }

            if (rx_idx >= 128) {
                rx_in_frame  = false;
                rx_idx       = 0;
                rx_frame_len = 0;
            }
        }
    }
}

void xbee_task(void *arg)
{
    while (1) {
        parse_rx();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}