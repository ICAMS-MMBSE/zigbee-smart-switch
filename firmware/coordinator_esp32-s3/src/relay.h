#pragma once

#include <stdint.h>
#include <stdbool.h>

void relay_led_init(void);
void relay_cmd(const char *node_id, uint8_t state);
void relay_toggle(const char *node_id);
void relay_poll(const char *node_id);
void relay_update_led(bool state);  // called when a joiner reports its relay state
