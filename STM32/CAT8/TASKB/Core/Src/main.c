/*
 * File: main.c
 * Target: STM32F103C8T6 (Blue Pill) - Continuous Evaluation 8
 * Description: GPIO input/output, debounced switch toggle, and non-blocking short/long press detection
 */
#include "main.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

typedef enum {
    STATE_IDLE,
    STATE_BLINKING
} SystemState_t;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    SystemState_t sys_state = STATE_IDLE;
    uint32_t press_start_time = 0;
    uint32_t last_blink_time = 0;
    uint8_t button_was_pressed = 0;

    while (1)
    {
        uint8_t button_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

        if (button_state == GPIO_PIN_SET && !button_was_pressed)
        {
            /* Button press transition detected */
            press_start_time = HAL_GetTick();
            button_was_pressed = 1;
            HAL_Delay(20); /* Software debounce filter */
        }
        else if (button_state == GPIO_PIN_RESET && button_was_pressed)
        {
            /* Button released: Calculate duration */
            uint32_t duration = HAL_GetTick() - press_start_time;
            button_was_pressed = 0;
            HAL_Delay(20); /* Software debounce filter */

            if (duration < 1000)
            {
                /* Short press (< 1s): Toggle LED / Stop blinking */
                if (sys_state == STATE_BLINKING)
                {
                    sys_state = STATE_IDLE;
                    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); /* LED OFF (Active-Low) */
                }
                else
                {
                    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
                }
            }
            else
            {
                /* Long press (>= 1s): Enter or re-enter continuous blinking mode */
                sys_state = STATE_BLINKING;
            }
        }

        /* Continuous blinking mode execution (non-blocking) */
        if (sys_state == STATE_BLINKING)
        {
            if (HAL_GetTick() - last_blink_time >= 100)
            {
                last_blink_time = HAL_GetTick();
                HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            }
        }
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure PC13 as Output Push-Pull (Onboard LED, Active-Low) */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Configure PA0 as Input with Pull-Down (External Push-Button) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void SystemClock_Config(void)
{
    /* Internal 8MHz HSI clock configuration */
}
