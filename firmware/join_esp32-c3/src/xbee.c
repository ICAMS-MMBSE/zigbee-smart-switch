#include "xbee.h"
#include "relay.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "XBEE";

// Commissioner address -- learned from the first packet we receive
static bool     comm_known = false;
static uint32_t comm_sh    = 0;
static uint32_t comm_sl    = 0;

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

// Build and send a ZBTxRequest (0x10) to a specific 64-bit address
static void xbee_send_raw(uint32_t sh, uint32_t sl, uint8_t cmd)
{
    uint8_t frame[19];

    frame[0]  = 0x7E;
    frame[1]  = 0x00;
    frame[2]  = 0x0F;           // payload length = 15
    frame[3]  = FRAME_TX_REQUEST;
    frame[4]  = 0x01;           // frame ID
    frame[5]  = (sh >> 24) & 0xFF;
    frame[6]  = (sh >> 16) & 0xFF;
    frame[7]  = (sh >>  8) & 0xFF;
    frame[8]  =  sh        & 0xFF;
    frame[9]  = (sl >> 24) & 0xFF;
    frame[10] = (sl >> 16) & 0xFF;
    frame[11] = (sl >>  8) & 0xFF;
    frame[12] =  sl        & 0xFF;
    frame[13] = 0xFF;           // 16-bit network address (unknown)
    frame[14] = 0xFE;
    frame[15] = 0x00;           // broadcast radius
    frame[16] = 0x00;           // options
    frame[17] = cmd;
    frame[18] = calc_checksum(frame, 19);

    uart_write_bytes(XBEE_UART, (const char *)frame, 19);
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

void xbee_report_state(bool state)
{
    if (!comm_known) {
        ESP_LOGW(TAG, "Commissioner unknown, skipping report");
        return;
    }
    xbee_send_raw(comm_sh, comm_sl, state ? CMD_ON : CMD_OFF);
    ESP_LOGI(TAG, "State reported: %s", state ? "ON" : "OFF");
}

static void parse_xbee(void)
{
    static uint8_t  buf[128];
    static uint8_t  idx       = 0;
    static uint16_t frame_len = 0;
    static bool     in_frame  = false;

    uint8_t b;
    while (uart_read_bytes(XBEE_UART, &b, 1, 0) == 1) {
        if (!in_frame) {
            if (b == 0x7E) {
                in_frame  = true;
                idx       = 0;
                frame_len = 0;
                buf[idx++] = b;
            }
        } else {
            buf[idx++] = b;

            if (idx == 3) {
                frame_len = ((uint16_t)buf[1] << 8) | buf[2];
            }

            if (frame_len > 0 && idx == (uint8_t)(frame_len + 4)) {
                if (verify_checksum(buf, idx)) {
                    uint8_t frame_type = buf[3];
                    ESP_LOGD(TAG, "Frame type: 0x%02X", frame_type);

                    if (frame_type == FRAME_ZB_RX) {
                        uint32_t src_sh = ((uint32_t)buf[4]  << 24) |
                                          ((uint32_t)buf[5]  << 16) |
                                          ((uint32_t)buf[6]  <<  8) |
                                           buf[7];
                        uint32_t src_sl = ((uint32_t)buf[8]  << 24) |
                                          ((uint32_t)buf[9]  << 16) |
                                          ((uint32_t)buf[10] <<  8) |
                                           buf[11];

                        // Learn the commissioner address from the first packet we see
                        if (!comm_known) {
                            comm_sh    = src_sh;
                            comm_sl    = src_sl;
                            comm_known = true;
                            ESP_LOGI(TAG, "Commissioner learned: SH=%08lX SL=%08lX",
                                     (unsigned long)comm_sh, (unsigned long)comm_sl);
                        }

                        // Payload starts at byte 15 (after 64-bit addr + 16-bit addr + options)
                        if (frame_len > 12) {
                            uint8_t cmd = buf[15];
                            ESP_LOGI(TAG, "Received cmd: 0x%02X", cmd);

                            switch (cmd) {
                                case CMD_ON:
                                    relay_set(true);
                                    xbee_report_state(true);
                                    break;
                                case CMD_OFF:
                                    relay_set(false);
                                    xbee_report_state(false);
                                    break;
                                case CMD_TOGGLE:
                                    relay_set(!relay_get_state());
                                    xbee_report_state(relay_get_state());
                                    break;
                                case CMD_POLL:
                                    xbee_report_state(relay_get_state());
                                    break;
                                default:
                                    ESP_LOGW(TAG, "Unknown cmd: 0x%02X", cmd);
                                    break;
                            }
                        }
                    }
                } else {
                    ESP_LOGW(TAG, "Checksum failed");
                }

                in_frame  = false;
                idx       = 0;
                frame_len = 0;
            }

            if (idx >= 128) {
                ESP_LOGW(TAG, "Buffer overflow, resetting");
                in_frame  = false;
                idx       = 0;
                frame_len = 0;
            }
        }
    }
}

void xbee_task(void *arg)
{
    while (1) {
        parse_xbee();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}