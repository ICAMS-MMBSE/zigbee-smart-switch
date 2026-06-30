#pragma once
#include "driver/gpio.h"
#include "driver/uart.h"

// Pin definitions
#define BTN_PIN     GPIO_NUM_3
#define XBEE_RX     GPIO_NUM_1
#define XBEE_TX     GPIO_NUM_0
// UART
#define XBEE_UART       UART_NUM_1
#define XBEE_BAUD       115200
#define UART_BUF_SIZE   256
// XBee command bytes
#define CMD_OFF    0x00
#define CMD_ON     0x01
#define CMD_TOGGLE 0x02
#define CMD_POLL   0x03
// Node table
#define MAX_NODES 10
// W5500 SPI pins
#define W5500_SCLK  GPIO_NUM_5
#define W5500_MOSI  GPIO_NUM_7
#define W5500_MISO  GPIO_NUM_8
#define W5500_CS    GPIO_NUM_6
#define W5500_INT   GPIO_NUM_9
#define W5500_RST   GPIO_NUM_10