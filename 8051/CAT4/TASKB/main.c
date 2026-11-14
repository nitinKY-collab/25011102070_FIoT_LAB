/*
 * File: main.c
 * Target: AT89S52 (Continuous Evaluation 4)
 * Description: Human-Machine Interface with 4x4 matrix keypad, 7-seg display, and status LEDs
 */
#include <reg51.h>
#include "keypad.h"
#include "seven_seg.h"

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++);
    }
}

void running_led(void) {
    unsigned char i;
    for (i = 0; i < 4; i++) {
        P1 = (P1 & 0xF0) | (1 << i);
        delay_ms(150);
    }
}

void binary_counter(void) {
    unsigned char c;
    for (c = 0; c < 16; c++) {
        P1 = (P1 & 0xF0) | (c & 0x0F);
        delay_ms(150);
    }
}

void alternate_led(void) {
    P1 = (P1 & 0xF0) | 0x0A;
    delay_ms(200);
    P1 = (P1 & 0xF0) | 0x05;
    delay_ms(200);
}

void blink_all(void) {
    P1 = (P1 & 0xF0) | 0x0F;
    delay_ms(200);
    P1 = (P1 & 0xF0) | 0x00;
    delay_ms(200);
}

void led_control(unsigned char key) {
    switch (key) {
        case 1: P1 = (P1 & 0xF0) | 0x01; break; /* LED1 ON */
        case 2: P1 = (P1 & 0xF0) | 0x02; break; /* LED2 ON */
        case 3: P1 = (P1 & 0xF0) | 0x04; break; /* LED3 ON */
        case 4: P1 = (P1 & 0xF0) | 0x08; break; /* LED4 ON */
        case 5: running_led(); break;
        case 6: alternate_led(); break;
        case 7: binary_counter(); break;
        case 8: blink_all(); break;
        case 9: P1 = (P1 & 0xF0) | 0x00; break; /* All OFF / Reset */
        default: break;
    }
}

void main(void) {
    unsigned char key;
    P0 = 0x00; /* Initialize Port 0 */
    P1 = 0x00; /* Initialize Port 1 */
    display_digit(0); /* Display '0' initially */
    
    while (1) {
        key = keypad_scan();
        if (key != 0xFF) {
            display_digit(key);
            led_control(key);
        }
    }
}
