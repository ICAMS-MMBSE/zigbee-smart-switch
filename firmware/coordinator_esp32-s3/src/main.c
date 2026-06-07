#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "ethernet.h"
#include "mqtt.h"
#include "xbee.h"
#include "discovery.h"
#include "relay.h"
#include "button.h"
#include "config.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "MAIN";

// Read newline-terminated commands from UART0 (USB serial) and dispatch them
static void console_task(void *arg)
{
    uart_config_t cfg = {
        .baud_rate  = 115200,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_driver_install(UART_NUM_0, 256 * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &cfg);

    uint8_t buf[128];
    uint8_t idx = 0;

    while (1) {
        uint8_t b;
        if (uart_read_bytes(UART_NUM_0, &b, 1, pdMS_TO_TICKS(10)) != 1) continue;

        if (b == '\n' || b == '\r') {
            if (idx == 0) continue;
            buf[idx] = '\0';
            char *cmd = (char *)buf;
            idx = 0;

            if (strcmp(cmd, "AT_PING") == 0) {
                xbee_ping();
            } else if (strcmp(cmd, "DISCOVER") == 0) {
                discover_nodes();
                print_node_table();
            } else if (strcmp(cmd, "NODES") == 0) {
                print_node_table();
            } else if (strncmp(cmd, "RELAY ", 6) == 0) {
                // Format: RELAY <nodeId> <0|1>
                char    node_id[16];
                uint8_t state;
                sscanf(cmd + 6, "%15s %hhu", node_id, &state);
                relay_cmd(node_id, state);
            } else if (strncmp(cmd, "TOGGLE ", 7) == 0) {
                relay_toggle(cmd + 7);
            } else if (strncmp(cmd, "POLL ", 5) == 0) {
                relay_poll(cmd + 5);
            } else {
                ESP_LOGW(TAG, "Unknown command: %s", cmd);
            }
        } else {
            if (idx < 127) buf[idx++] = b;
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Commissioner starting...");
    ethernet_init();
    button_init();
    relay_led_init();
    xbee_init();

    vTaskDelay(pdMS_TO_TICKS(2000)); // let XBee3 boot
    discover_nodes();
    print_node_table();

    vTaskDelay(pdMS_TO_TICKS(3000)); // give joiners time to settle before polling
    relay_poll("ALL");

    xTaskCreate(xbee_task,    "xbee_task",    4096, NULL, 5, NULL);
    xTaskCreate(button_task,  "button_task",  2048, NULL, 5, NULL);
    xTaskCreate(console_task, "console_task", 4096, NULL, 4, NULL);

    mqtt_init();
    ESP_LOGI(TAG, "Commissioner ready");
}
