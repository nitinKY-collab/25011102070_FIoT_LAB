/*
 * File: main.c
 * Target: STM32 NUCLEO-F401RE - Continuous Evaluation 11
 * Description: GPIO output on User LED (PA5 LD2) and input on Push Button (PC13 B1) with short/long press
 */
#include "main.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    uint32_t press_time = 0;
    uint8_t btn_pressed = 0;
    uint8_t alert_mode = 0;
    uint32_t last_alert_toggle = 0;

    while (1)
    {
        /* Button B1 on PC13 is Active-Low */
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET && !btn_pressed)
        {
            press_time = HAL_GetTick();
            btn_pressed = 1;
            HAL_Delay(25); /* Debounce */
        }
        else if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET && btn_pressed)
        {
            uint32_t duration = HAL_GetTick() - press_time;
            btn_pressed = 0;
            HAL_Delay(25); /* Debounce */

            if (duration < 1000)
            {
                /* Short press: Toggle LED LD2 and disable alert mode if active */
                alert_mode = 0;
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            }
            else
            {
                /* Long press: Trigger rapid alert flashing mode */
                alert_mode = 1;
            }
        }

        if (alert_mode)
        {
            if (HAL_GetTick() - last_alert_toggle >= 80)
            {
                last_alert_toggle = HAL_GetTick();
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            }
        }
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configure PA5 as Output (Green User LED LD2, Active-High) */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure PC13 as Input (Blue User Button B1, Active-Low) */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL; /* Onboard pull-up present */
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void SystemClock_Config(void) {}
