#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "relay.h"
#include "button.h"
#include "xbee.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Joiner starting...");

    relay_init();
    button_init();
    xbee_init();

    xTaskCreate(xbee_task,   "xbee_task",   4096, NULL, 5, NULL);
    xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);

    ESP_LOGI(TAG, "Joiner ready");
}