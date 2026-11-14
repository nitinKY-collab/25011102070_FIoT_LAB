/*
 * File: main.c
 * Target: AT89S52 (Continuous Evaluation 5)
 * Description: Hardware Timer 0/1 periodic interrupts for PWM LED fading and stepper motor sequencing
 */
#include <reg51.h>
#include "timer.h"

sbit PWM_LED = P1^0;

/* Stepper motor sequence (P1.4-P1.7 connected to ULN2003) */
static const unsigned char step_sequence[4] = {
    0x10, /* 0001 0000 */
    0x20, /* 0010 0000 */
    0x40, /* 0100 0000 */
    0x80  /* 1000 0000 */
};

volatile unsigned int tick_count = 0;
volatile unsigned char pwm_counter = 0;
volatile unsigned char duty_cycle = 0;
volatile unsigned char step_index = 0;
volatile unsigned int stepper_delay_counter = 0;

/* Timer 0 ISR: Generates 1ms time base */
void timer0_isr(void) interrupt 1 {
    TH0 = 0xFC; /* Preload for 1ms @ 11.0592 MHz */
    TL0 = 0x66;
    
    tick_count++;
    stepper_delay_counter++;
    
    /* Software PWM handling */
    pwm_counter = (pwm_counter + 1) % 100;
    if (pwm_counter < duty_cycle) {
        PWM_LED = 1;
    } else {
        PWM_LED = 0;
    }
    
    /* Stepper motor stepping every 50ms */
    if (stepper_delay_counter >= 50) {
        stepper_delay_counter = 0;
        step_index = (step_index + 1) % 4;
        P1 = (P1 & 0x0F) | step_sequence[step_index];
    }
}

void main(void) {
    char fading_direction = 1;
    timer0_init_1ms();
    
    while (1) {
        /* Every 20ms update duty cycle to produce smooth fading */
        if (tick_count >= 20) {
            tick_count = 0;
            if (fading_direction == 1) {
                duty_cycle += 2;
                if (duty_cycle >= 100) {
                    duty_cycle = 100;
                    fading_direction = -1;
                }
            } else {
                duty_cycle -= 2;
                if (duty_cycle <= 0) {
                    duty_cycle = 0;
                    fading_direction = 1;
                }
            }
        }
    }
}
