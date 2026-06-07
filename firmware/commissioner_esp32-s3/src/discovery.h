#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_NODES 10

typedef struct {
    char     node_id[16];
    uint32_t addr_sh;
    uint32_t addr_sl;
    bool     active;
} ZigbeeNode;

extern ZigbeeNode nodeTable[MAX_NODES];
extern uint8_t    nodeCount;

void discover_nodes(void);
bool node_get_addr(const char *node_id, uint32_t *sh, uint32_t *sl);
void print_node_table(void);
