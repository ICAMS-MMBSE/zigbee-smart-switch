#pragma once

#include "driver/gpio.h"
#include "driver/uart.h"

// Pin definitions
#define RELAY_PIN   GPIO_NUM_2
#define BTN_PIN     GPIO_NUM_20
#define XBEE_RX     GPIO_NUM_1
#define XBEE_TX     GPIO_NUM_0

// UART
#define XBEE_UART       UART_NUM_1
#define XBEE_BAUD       115200
#define UART_BUF_SIZE   256

// XBee command bytes sent from the commissioner
#define CMD_OFF     0x00
#define CMD_ON      0x01
#define CMD_TOGGLE  0x02
#define CMD_POLL    0x03