#pragma once

#include "driver/gpio.h"
#include "driver/uart.h"

// Pin definitions
#define LED_PIN     GPIO_NUM_16
#define BTN_PIN     GPIO_NUM_15
#define XBEE_RX     GPIO_NUM_6
#define XBEE_TX     GPIO_NUM_7

// UART
#define XBEE_UART       UART_NUM_1
#define XBEE_BAUD       115200
#define UART_BUF_SIZE   256

// XBee command bytes
#define CMD_OFF    0x00
#define CMD_ON     0x01
#define CMD_TOGGLE 0x02
#define CMD_POLL   0x03
//#define CMD_HELLO  0x04

// Node table
#define MAX_NODES 10