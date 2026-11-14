/*
 * File: main.c
 * Target: AT89S52 (Continuous Evaluation 2)
 * Description: Hardware bring-up and bare-metal LED blink with modular delay library
 */
#include <reg51.h>
#include "delay.h"

sbit USER_LED = P1^0;

void main(void) {
    USER_LED = 0;
    while (1) {
        USER_LED = 1;
        delay_ms(500);
        USER_LED = 0;
        delay_ms(500);
    }
}
