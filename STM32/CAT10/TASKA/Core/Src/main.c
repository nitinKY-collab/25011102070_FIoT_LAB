/*
 * File: main.c
 * Target: STM32F103C8T6 (Blue Pill) - Continuous Evaluation 10
 * Description: USART2 AT command interface communicating with ESP8266 Wemos D1 Mini
 */
#include "main.h"
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

void Wemos_SendCommand(char *cmd) {
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), 1000);
    HAL_Delay(500);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    uint8_t rx_buf[256] = {0};

    HAL_Delay(2000); /* Wait for Wemos D1 boot */

    /* Task 9A: Test basic communication */
    Wemos_SendCommand("AT\r\n");

    /* Task 9B: Explore AT commands */
    Wemos_SendCommand("AT+GMR\r\n");    /* Query version */
    Wemos_SendCommand("AT+CWMODE=1\r\n"); /* Station mode */
    Wemos_SendCommand("AT+CWLAP\r\n");   /* Scan APs */

    /* Task 9C: Connect to Wi-Fi Network */
    Wemos_SendCommand("AT+CWJAP=\"SNUC\",\"snu12345\"\r\n");
    HAL_Delay(5000); /* Wait for Wi-Fi association */
    Wemos_SendCommand("AT+CIFSR\r\n");   /* Get assigned IP */

    while (1)
    {
        /* Loop reading any incoming telemetry */
        if (HAL_UART_Receive(&huart2, rx_buf, sizeof(rx_buf)-1, 100) == HAL_OK)
        {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        }
    }
}

static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void SystemClock_Config(void) {}
