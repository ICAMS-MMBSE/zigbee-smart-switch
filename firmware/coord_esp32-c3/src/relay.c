#include "relay.h"
#include "discovery.h"
#include "xbee.h"
#include "mqtt.h"
#include "config.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <string.h>

static const char *TAG = "RELAY";

// The status LED on the commissioner mirrors the last known relay state from the joiner
void relay_led_init(void)
{
    gpio_config_t cfg = {
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);
}

static void send_to_all(uint8_t cmd)
{
    for (uint8_t i = 0; i < nodeCount; i++) {
        if (nodeTable[i].active) {
            xbee_send_raw(nodeTable[i].addr_sh, nodeTable[i].addr_sl, cmd);
        }
    }
}

static bool send_to_node(const char *node_id, uint8_t cmd)
{
    uint32_t sh, sl;
    if (node_get_addr(node_id, &sh, &sl)) {
        xbee_send_raw(sh, sl, cmd);
        return true;
    }
    ESP_LOGW(TAG, "Node not found: %s", node_id);
    return false;
}

void relay_cmd(const char *node_id, uint8_t state)
{
    uint8_t cmd = state ? CMD_ON : CMD_OFF;
    if (strcmp(node_id, "ALL") == 0) {
        send_to_all(cmd);
        ESP_LOGI(TAG, "ALL RELAY -> %s", state ? "ON" : "OFF");
    } else {
        if (send_to_node(node_id, cmd))
            ESP_LOGI(TAG, "%s RELAY -> %s", node_id, state ? "ON" : "OFF");
    }
}

void relay_toggle(const char *node_id)
{
    if (strcmp(node_id, "ALL") == 0) {
        send_to_all(CMD_TOGGLE);
        ESP_LOGI(TAG, "ALL RELAY TOGGLE");
    } else {
        if (send_to_node(node_id, CMD_TOGGLE))
            ESP_LOGI(TAG, "%s RELAY TOGGLE", node_id);
    }
}

void relay_poll(const char *node_id)
{
    if (strcmp(node_id, "ALL") == 0) {
        send_to_all(CMD_POLL);
        ESP_LOGI(TAG, "Polling ALL...");
    } else {
        if (send_to_node(node_id, CMD_POLL))
            ESP_LOGI(TAG, "Polling %s...", node_id);
    }
}