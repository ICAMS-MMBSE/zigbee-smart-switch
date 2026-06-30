#include "mqtt.h"
#include "relay.h"
#include "config.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t client = NULL;

static void mqtt_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");
            esp_mqtt_client_subscribe(client, "switch/+/cmd", 0);
            ESP_LOGI(TAG, "Subscribed to switch/+/cmd");
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT disconnected");
            break;

        case MQTT_EVENT_DATA: {
            char topic[32]   = {0};
            char payload[16] = {0};
            int tlen = event->topic_len < (int)sizeof(topic) - 1 ? event->topic_len : (int)sizeof(topic) - 1;
            int plen = event->data_len  < (int)sizeof(payload) - 1 ? event->data_len  : (int)sizeof(payload) - 1;
            memcpy(topic, event->topic, tlen);
            memcpy(payload, event->data, plen);

            ESP_LOGI(TAG, "Topic: %s  Data: %s", topic, payload);

            char node[16] = {0};
            if (sscanf(topic, "switch/%15[^/]/cmd", node) != 1) {
                ESP_LOGW(TAG, "Unparseable topic: %s", topic);
                break;
            }

            const char *target = (strcmp(node, "all") == 0) ? "ALL" : node;

            if (strcmp(payload, "ON") == 0) {
                relay_cmd(target, 1);
            } else if (strcmp(payload, "OFF") == 0) {
                relay_cmd(target, 0);
            } else if (strcmp(payload, "TOGGLE") == 0) {
                relay_toggle(target);
            } else if (strcmp(payload, "POLL") == 0) {
                relay_poll(target);
            } else {
                ESP_LOGW(TAG, "Unknown command: %s", payload);
            }
            break;
        }

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT error");
            break;

        default:
            break;
    }
}

void mqtt_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://rpi3.local:1883",
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    ESP_LOGI(TAG, "MQTT client started");
}

void mqtt_publish_state(const char *node_id, bool state)
{
    if (client == NULL) return;
    char topic[32];
    snprintf(topic, sizeof(topic), "switch/%s/state", node_id);
    const char *payload = state ? "ON" : "OFF";
    esp_mqtt_client_publish(client, topic, payload, 0, 0, 0);
    ESP_LOGI(TAG, "Published %s -> %s", topic, payload);
}