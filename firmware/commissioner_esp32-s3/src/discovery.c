#include "discovery.h"
#include "xbee.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "DISCOVERY";

ZigbeeNode nodeTable[MAX_NODES];
uint8_t    nodeCount = 0;

static void parse_nd_response(uint8_t *buf, uint8_t len)
{
    if (len < 10) return;
    if (nodeCount >= MAX_NODES) return;

    uint32_t sh = ((uint32_t)buf[2] << 24) | ((uint32_t)buf[3] << 16) |
                  ((uint32_t)buf[4] << 8)  |  buf[5];
    uint32_t sl = ((uint32_t)buf[6] << 24) | ((uint32_t)buf[7] << 16) |
                  ((uint32_t)buf[8] << 8)  |  buf[9];

    // Skip duplicates (XBee can send repeated ND responses)
    for (uint8_t i = 0; i < nodeCount; i++) {
        if (nodeTable[i].addr_sh == sh && nodeTable[i].addr_sl == sl) return;
    }

    // Parse the NI (Node Identifier) string -- null-terminated in the payload
    char    ni[16] = {0};
    uint8_t ni_len = 0;
    for (uint8_t i = 10; i < len && buf[i] != 0 && ni_len < 15; i++) {
        ni[ni_len++] = (char)buf[i];
    }

    nodeTable[nodeCount].addr_sh = sh;
    nodeTable[nodeCount].addr_sl = sl;
    nodeTable[nodeCount].active  = true;
    strncpy(nodeTable[nodeCount].node_id, ni, 15);
    nodeCount++;

    ESP_LOGI(TAG, "Found node: %s SH: %08lX SL: %08lX",
             ni, (unsigned long)sh, (unsigned long)sl);
}

void discover_nodes(void)
{
    nodeCount = 0;
    memset(nodeTable, 0, sizeof(nodeTable));
    ESP_LOGI(TAG, "Starting network discovery...");

    // Send ND (Node Discover) AT command frame
    uint8_t frame[8] = {
        0x7E, 0x00, 0x04,
        0x08, 0x01,
        'N',  'D',
        0x00
    };
    uint8_t checksum = 0;
    for (uint8_t i = 3; i <= 6; i++) checksum += frame[i];
    frame[7] = 0xFF - checksum;
    uart_write_bytes(XBEE_UART, (const char *)frame, 8);

    // Collect AT Response (0x88) frames for 6 seconds -- XBee sends one per discovered node
    uint8_t  buf[64];
    uint8_t  idx       = 0;
    uint16_t frame_len = 0;
    bool     in_frame  = false;
    uint32_t start     = xTaskGetTickCount();

    while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(6000)) {
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
                uint8_t sum = 0;
                for (uint16_t i = 3; i < idx - 1; i++) sum += buf[i];
                if ((0xFF - sum) == buf[idx - 1]) {
                    if (buf[3] == FRAME_AT_RESP && buf[7] == 0x00) {
                        uint8_t val_len = frame_len - 5;
                        parse_nd_response(&buf[8], val_len);
                    }
                }
                in_frame  = false;
                idx       = 0;
                frame_len = 0;
            }
            if (idx >= 64) {
                in_frame = false;
                idx = 0;
            }
        }
    }

    ESP_LOGI(TAG, "Discovery complete. Found %d nodes.", nodeCount);
}

bool node_get_addr(const char *node_id, uint32_t *sh, uint32_t *sl)
{
    for (uint8_t i = 0; i < nodeCount; i++) {
        if (nodeTable[i].active && strcmp(nodeTable[i].node_id, node_id) == 0) {
            *sh = nodeTable[i].addr_sh;
            *sl = nodeTable[i].addr_sl;
            return true;
        }
    }
    return false;
}

void print_node_table(void)
{
    ESP_LOGI(TAG, "=== Node Table ===");
    for (uint8_t i = 0; i < nodeCount; i++) {
        ESP_LOGI(TAG, "%s -> SH: %08lX SL: %08lX",
                 nodeTable[i].node_id,
                 (unsigned long)nodeTable[i].addr_sh,
                 (unsigned long)nodeTable[i].addr_sl);
    }
    ESP_LOGI(TAG, "==================");
}
