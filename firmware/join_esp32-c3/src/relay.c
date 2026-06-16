#include "relay.h"
#include "config.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "RELAY";
static bool relay_state = false;

void relay_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << RELAY_PIN),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);
    gpio_set_level(RELAY_PIN, 0);
    ESP_LOGI(TAG, "Relay init done, defaulting OFF");
}

void relay_set(bool state)
{
    relay_state = state;
    gpio_set_level(RELAY_PIN, state);
    ESP_LOGI(TAG, "Relay: %s", state ? "ON" : "OFF");
}

bool relay_get_state(void)
{
    return relay_state;
}
