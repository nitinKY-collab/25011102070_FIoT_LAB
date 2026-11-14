/*
 * File: main.c
 * Target: STM32 NUCLEO-F401RE - Continuous Evaluation 13
 * Description: BME280 environmental telemetry published to public MQTT broker (broker.emqx.io) via ESP8266 ESPLib
 */
#include "main.h"
#include "bme280.h"
#include "ESPLib.h"
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

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();

    /* 1. BME280 Sensor Initialization */
    static uint8_t dev_addr = BME280_I2C_ADDR_PRIM;
    dev.intf_ptr = &dev_addr;
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;

    bme280_init(&dev);
    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_1X;
    dev.settings.osr_t = BME280_OVERSAMPLING_1X;
    bme280_set_sensor_settings(BME280_SEL_OSR_PRESS | BME280_SEL_OSR_TEMP | BME280_SEL_OSR_HUM, &dev);
    bme280_set_sensor_mode(BME280_MODE_NORMAL, &dev);

    /* 2. ESP8266 MQTT Driver Initialization */
    ESP8266_Init(StationMode, huart2);
    ESP8266_wifiConnect("SNUC", "snu12345");
    ESP8266_portConnect("TCP", "broker.emqx.io", "1883");

    MQTT_InitTypeDef_t mqttSetting = {0};
    mqttSetting.keepAlive = 500;
    mqttSetting.connectFlag = CleanSession;
    strcpy(mqttSetting.clientID, "STM32_Publisher_25011102070");
    MQTT_connectBroker(&mqttSetting);

    char json_payload[180];

    while (1)
    {
        /* Read sensor measurements */
        bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
        temperature = comp_data.temperature;
        pressure = comp_data.pressure / 100.0f;
        humidity = comp_data.humidity;

        /* Format JSON payload */
        sprintf(json_payload, "{\"status\":\"success\",\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}", temperature, humidity, pressure);

        /* Publish telemetry to MQTT topic */
        MQTT_publishTopic("stm32/ky/data", json_payload, QoS0);

        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(1000);
    }
}

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
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

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void SystemClock_Config(void) {}
