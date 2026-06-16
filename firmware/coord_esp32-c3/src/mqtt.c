#include "mqtt.h"
#include "relay.h"
#include "config.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <string.h>

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t client = NULL;

static void mqtt_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");
            esp_mqtt_client_subscribe(client, "switch/all/cmd", 0);
            esp_mqtt_client_subscribe(client, "switch/1/cmd", 0);
            ESP_LOGI(TAG, "Subscribed to command topics");
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT disconnected");
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Topic: %.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "Data:  %.*s", event->data_len, event->data);

        if (strncmp(event->data, "ON", event->data_len) == 0) {
            relay_cmd("ALL", 1);
        } else if (strncmp(event->data, "OFF", event->data_len) == 0) {
            relay_cmd("ALL", 0);
        } else if (strncmp(event->data, "TOGGLE", event->data_len) == 0) {
            relay_toggle("ALL");
        } else if (strncmp(event->data, "POLL", event->data_len) == 0) {
            relay_poll("ALL");
        }
            break;

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
        .broker.address.uri = "mqtt://192.168.2.1:1883",
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