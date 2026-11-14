#include "delay.h"

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++) {
            /* 1 ms delay calibrated for 11.0592 MHz crystal */
        }
    }
}

void delay_us(unsigned int us) {
    unsigned int i;
    for (i = 0; i < us; i++) {
        /* Short delay loop */
    }
}
