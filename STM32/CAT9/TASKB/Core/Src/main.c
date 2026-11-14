/*
 * File: main.c
 * Target: STM32F103C8T6 (Blue Pill) - Continuous Evaluation 9
 * Description: USART1 command-line interface controlling onboard LED (LED ON, LED OFF, LED TOGGLE)
 */
#include "main.h"
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

void UART_SendString(char *str) {
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    char rx_buf[64];
    uint8_t rx_idx = 0;
    uint8_t ch;

    UART_SendString("\r\n=== STM32 UART COMMAND CONTROLLER ===\r\n");
    UART_SendString("Commands: 'LED ON', 'LED OFF', 'LED TOGGLE'\r\n> ");

    while (1)
    {
        if (HAL_UART_Receive(&huart1, &ch, 1, 10) == HAL_OK)
        {
            /* Echo character back to terminal */
            HAL_UART_Transmit(&huart1, &ch, 1, 10);

            if (ch == '\r' || ch == '\n')
            {
                if (rx_idx > 0)
                {
                    rx_buf[rx_idx] = '\0';
                    UART_SendString("\r\n");

                    if (strcmp(rx_buf, "LED ON") == 0)
                    {
                        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); /* Active-Low ON */
                        UART_SendString("LED turned ON\r\n> ");
                    }
                    else if (strcmp(rx_buf, "LED OFF") == 0)
                    {
                        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); /* Active-Low OFF */
                        UART_SendString("LED turned OFF\r\n> ");
                    }
                    else if (strcmp(rx_buf, "LED TOGGLE") == 0)
                    {
                        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
                        UART_SendString("LED toggled\r\n> ");
                    }
                    else
                    {
                        UART_SendString("Unknown command. Try: LED ON, LED OFF, LED TOGGLE\r\n> ");
                    }
                    rx_idx = 0;
                }
            }
            else if (rx_idx < sizeof(rx_buf) - 1)
            {
                rx_buf[rx_idx++] = ch;
            }
        }
    }
}

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* PC13 LED */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void SystemClock_Config(void) {}
