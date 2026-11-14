/*
 * File: main.c
 * Target: STM32F103C8T6 (Blue Pill) - Continuous Evaluation 10C
 * Description: Embedded HTTP web server on STM32 via ESP8266 AT commands serving web clients & Node-RED
 */
#include "main.h"
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

void ESP_SendCommand(char *cmd) {
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), 1000);
    HAL_Delay(300);
}

void Serve_HTTP_Response(int link_id) {
    char http_body[] = "<html><body><h1>STM32 IoT Node</h1><p>Hello from STM32 Blue Pill!</p></body></html>";
    int body_len = strlen(http_body);
    char http_header[256];
    char send_cmd[64];
    
    int header_len = sprintf(http_header, 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s", body_len, http_body);
        
    sprintf(send_cmd, "AT+CIPSEND=%d,%d\r\n", link_id, header_len);
    HAL_UART_Transmit(&huart2, (uint8_t*)send_cmd, strlen(send_cmd), 500);
    HAL_Delay(50);
    HAL_UART_Transmit(&huart2, (uint8_t*)http_header, header_len, 1000);
    HAL_Delay(100);
    
    char close_cmd[32];
    sprintf(close_cmd, "AT+CIPCLOSE=%d\r\n", link_id);
    HAL_UART_Transmit(&huart2, (uint8_t*)close_cmd, strlen(close_cmd), 500);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    HAL_Delay(2000);

    /* 1. Setup Multi-connection and TCP Server on Port 80 */
    ESP_SendCommand("AT\r\n");
    ESP_SendCommand("AT+CIPMUX=1\r\n");
    ESP_SendCommand("AT+CIPSERVER=1,80\r\n");

    uint8_t rx_buf[512];

    while (1)
    {
        memset(rx_buf, 0, sizeof(rx_buf));
        if (HAL_UART_Receive(&huart2, rx_buf, sizeof(rx_buf)-1, 500) == HAL_OK)
        {
            /* Check for +IPD frame: +IPD,<link_id>,<len>:GET / HTTP/1.1 */
            char *ipd_ptr = strstr((char*)rx_buf, "+IPD,");
            if (ipd_ptr != NULL)
            {
                int link_id = 0;
                sscanf(ipd_ptr, "+IPD,%d,", &link_id);
                Serve_HTTP_Response(link_id);
                HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            }
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
