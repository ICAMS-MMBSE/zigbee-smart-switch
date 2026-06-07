#pragma once
#include <stdbool.h>

void mqtt_init(void);
void mqtt_publish_state(const char *node_id, bool state);