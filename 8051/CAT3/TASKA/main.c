/*
 * File: main.c
 * Target: AT89S52 (Continuous Evaluation 3)
 * Description: Multi-pattern LED generator with debounced button state machine
 */
#include <reg51.h>

sbit BUTTON = P3^2; /* INT0 pin used for mode selection */

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++);
    }
}

void mode_running_led(void) {
    unsigned char pattern = 0x01;
    unsigned char k;
    for (k = 0; k < 4; k++) {
        P1 = (P1 & 0xF0) | (pattern & 0x0F);
        delay_ms(200);
        pattern <<= 1;
        if (!BUTTON) return;
    }
}

void mode_binary_counter(void) {
    unsigned char count;
    for (count = 0; count < 16; count++) {
        P1 = (P1 & 0xF0) | (count & 0x0F);
        delay_ms(250);
        if (!BUTTON) return;
    }
}

void mode_alternating(void) {
    P1 = (P1 & 0xF0) | 0x0A; /* 1010 */
    delay_ms(300);
    if (!BUTTON) return;
    P1 = (P1 & 0xF0) | 0x05; /* 0101 */
    delay_ms(300);
}

void mode_flash_all(void) {
    P1 = (P1 & 0xF0) | 0x0F; /* 1111 */
    delay_ms(300);
    if (!BUTTON) return;
    P1 = (P1 & 0xF0) | 0x00; /* 0000 */
    delay_ms(300);
}

void main(void) {
    unsigned char mode = 0;
    BUTTON = 1; /* Configure P3.2 as input */
    P1 = P1 & 0xF0; /* LEDs initially OFF */
    
    while (1) {
        if (BUTTON == 0) {
            delay_ms(20); /* Software debounce filter */
            if (BUTTON == 0) {
                mode = (mode + 1) % 4;
                while (BUTTON == 0); /* Wait for button release */
                delay_ms(20);
            }
        }
        
        switch (mode) {
            case 0: mode_running_led(); break;
            case 1: mode_binary_counter(); break;
            case 2: mode_alternating(); break;
            case 3: mode_flash_all(); break;
        }
    }
}
