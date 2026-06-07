#include "button.h"
#include "relay.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "BUTTON";

void button_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << BTN_PIN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);
    ESP_LOGI(TAG, "Button init done");
}

void button_task(void *arg)
{
    bool last_state = true; // HIGH at rest (active-low button with pull-up)

    while (1) {
        bool current = gpio_get_level(BTN_PIN);

        if (current != last_state) {
            vTaskDelay(pdMS_TO_TICKS(100)); // debounce
            current = gpio_get_level(BTN_PIN);
            if (current == 0) {
                ESP_LOGI(TAG, "Button pressed, toggling ALL");
                relay_toggle("ALL");
            }
            last_state = current;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
