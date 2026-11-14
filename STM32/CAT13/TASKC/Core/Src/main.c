/*
 * File: main.c
 * Target: STM32 NUCLEO-F401RE - Continuous Evaluation 12
 * Description: BME280 I2C sensor driver + ESP8266 HTTP server streaming JSON telemetry to Node-RED
 */
#include "main.h"
#include "bme280.h"
#include <stdio.h>
#include <string.h>

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;

struct bme280_dev dev;
struct bme280_data comp_data;
int8_t rslt = BME280_OK;

float temperature, pressure, humidity;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    uint8_t dev_addr = *(uint8_t*)intf_ptr;
    return (HAL_I2C_Mem_Read(&hi2c1, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, len, 1000) == HAL_OK) ? BME280_OK : BME280_E_COMM_FAIL;
}

int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    uint8_t dev_addr = *(uint8_t*)intf_ptr;
    return (HAL_I2C_Mem_Write(&hi2c1, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)reg_data, len, 1000) == HAL_OK) ? BME280_OK : BME280_E_COMM_FAIL;
}

void user_delay_us(uint32_t period, void *intf_ptr) {
    uint32_t i;
    for (i = 0; i < (period * 10); i++) __NOP();
}

void Serve_JSON_HTTP(int link_id) {
    char json_payload[180];
    sprintf(json_payload, "{\"status\":\"success\",\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}", temperature, humidity, pressure);
    
    char http_resp[320];
    int resp_len = sprintf(http_resp,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s", (int)strlen(json_payload), json_payload);
        
    char send_cmd[32];
    sprintf(send_cmd, "AT+CIPSEND=%d,%d\r\n", link_id, resp_len);
    HAL_UART_Transmit(&huart2, (uint8_t*)send_cmd, strlen(send_cmd), 500);
    HAL_Delay(50);
    HAL_UART_Transmit(&huart2, (uint8_t*)http_resp, resp_len, 1000);
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
    MX_I2C1_Init();
    MX_USART2_UART_Init();

    /* BME280 Sensor Initialization */
    static uint8_t dev_addr = BME280_I2C_ADDR_PRIM; /* 0x76 */
    dev.intf_ptr = &dev_addr;
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;

    rslt = bme280_init(&dev);

    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_1X;
    dev.settings.osr_t = BME280_OVERSAMPLING_1X;
    dev.settings.filter = BME280_FILTER_COEFF_OFF;
    bme280_set_sensor_settings(BME280_SEL_OSR_PRESS | BME280_SEL_OSR_TEMP | BME280_SEL_OSR_HUM, &dev);
    bme280_set_sensor_mode(BME280_MODE_NORMAL, &dev);

    /* Initialize ESP8266 HTTP Server */
    HAL_Delay(1000);
    HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CIPMUX=1\r\n", 13, 500);
    HAL_Delay(200);
    HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CIPSERVER=1,80\r\n", 19, 500);
    HAL_Delay(200);

    uint8_t rx_buf[256];

    while (1)
    {
        /* Periodic Sensor Reading */
        bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
        temperature = comp_data.temperature;
        pressure = comp_data.pressure / 100.0f; /* Convert Pa to hPa */
        humidity = comp_data.humidity;

        /* Check for HTTP Request from Node-RED */
        memset(rx_buf, 0, sizeof(rx_buf));
        if (HAL_UART_Receive(&huart2, rx_buf, sizeof(rx_buf)-1, 200) == HAL_OK)
        {
            char *p = strstr((char*)rx_buf, "+IPD,");
            if (p != NULL)
            {
                int link_id = 0;
                sscanf(p, "+IPD,%d,", &link_id);
                Serve_JSON_HTTP(link_id);
            }
        }
    }
}

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    HAL_I2C_Init(&hi2c1);
}

static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    HAL_UART_Init(&huart2);
}

static void MX_GPIO_Init(void) {}
void SystemClock_Config(void) {}
