
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bme280.h"
#include <stdio.h>
#include <string.h>
#include "ESPLib.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
extern I2C_HandleTypeDef hi2c1;

struct bme280_dev dev;
struct bme280_data comp_data;

int8_t rslt = BME280_OK;

float temperature;
float pressure;
float humidity;

char mqttMessage[150];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART6_UART_Init(void);

/* USER CODE BEGIN PFP */
int8_t user_i2c_read(uint8_t reg_addr,
                     uint8_t *reg_data,
                     uint32_t len,
                     void *intf_ptr);

int8_t user_i2c_write(uint8_t reg_addr,
                      const uint8_t *reg_data,
                      uint32_t len,
                      void *intf_ptr);

void user_delay_us(uint32_t period, void *intf_ptr);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART6_UART_Init();

    /* USER CODE BEGIN 2 */

    static uint8_t dev_addr = BME280_I2C_ADDR_PRIM;

    dev.intf_ptr = &dev_addr;
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;

    /* Initialize BME280 */
    rslt = bme280_init(&dev);

    /* Configure BME280 */
    struct bme280_settings settings = {0};

    settings.osr_h = BME280_OVERSAMPLING_1X;
    settings.osr_p = BME280_OVERSAMPLING_1X;
    settings.osr_t = BME280_OVERSAMPLING_1X;
    settings.filter = BME280_FILTER_COEFF_OFF;

    rslt = bme280_set_sensor_settings(
        BME280_SEL_OSR_PRESS |
        BME280_SEL_OSR_TEMP |
        BME280_SEL_OSR_HUM |
        BME280_SEL_FILTER,
        &settings,
        &dev
    );

    /* Set BME280 to normal mode */
    rslt = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &dev);

    /* Initialize ESP8266 */
    ESP8266_Init(StationMode, huart6);

    /* Connect ESP8266 to WiFi */
    ESP8266_wifiConnect("SNUC", "snu12345");

    /* Connect to MQTT broker */
    ESP8266_portConnect("TCP", "broker.emqx.io", "1883");

    /* MQTT Configuration */
    MQTT_InitTypeDef_t mqttSetting;

    mqttSetting.keepAlive = 500;
    mqttSetting.connectFlag = CleanSession;
    mqttSetting.clientID = "STM32_Publisher";

    /* Connect to MQTT broker */
    MQTT_connectBroker(&mqttSetting);

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);

        if (rslt == BME280_OK)
        {
            temperature = comp_data.temperature;
            pressure = comp_data.pressure / 100.0f;
            humidity = comp_data.humidity;

            snprintf(mqttMessage, sizeof(mqttMessage),
                     "{\"temperature\":%.2f,\"pressure\":%.2f,\"humidity\":%.2f}",
                     temperature,
                     pressure,
                     humidity);

            MQTT_publishTopic("stm32/shakthi/data", mqttMessage, QoS0);
        }

        HAL_Delay(2000);

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 84;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 |
                                  RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief I2C1 Initialization Function
  */
static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief USART6 Initialization Function
  */
static void MX_USART6_UART_Init(void)
{
    huart6.Instance = USART6;
    huart6.Init.BaudRate = 115200;
    huart6.Init.WordLength = UART_WORDLENGTH_8B;
    huart6.Init.StopBits = UART_STOPBITS_1;
    huart6.Init.Parity = UART_PARITY_NONE;
    huart6.Init.Mode = UART_MODE_TX_RX;
    huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart6.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart6) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

/* USER CODE BEGIN 4 */

/* Read BME280 registers */
int8_t user_i2c_read(uint8_t reg_addr,
                     uint8_t *reg_data,
                     uint32_t len,
                     void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t *)intf_ptr;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        &hi2c1,
        (uint16_t)(dev_addr << 1),
        reg_addr,
        I2C_MEMADD_SIZE_8BIT,
        reg_data,
        len,
        1000
    );

    if (status == HAL_OK)
    {
        return BME280_OK;
    }

    return BME280_E_COMM_FAIL;
}

/* Write BME280 registers */
int8_t user_i2c_write(uint8_t reg_addr,
                      const uint8_t *reg_data,
                      uint32_t len,
                      void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t *)intf_ptr;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
        &hi2c1,
        (uint16_t)(dev_addr << 1),
        reg_addr,
        I2C_MEMADD_SIZE_8BIT,
        (uint8_t *)reg_data,
        len,
        1000
    );

    if (status == HAL_OK)
    {
        return BME280_OK;
    }

    return BME280_E_COMM_FAIL;
}

/* BME280 delay function */
void user_delay_us(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;

    HAL_Delay((period / 1000U) +
              ((period % 1000U) != 0U));
}

/* Library timer callback */
void HAL_SYSTICK_Callback(void)
{
    MQTTTimer();
}

/* USER CODE END 4 */

/**
  * @brief Error Handler
  */
void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
