/*
 * File: main.c
 * Target: AT89S52 (Continuous Evaluation 7)
 * Description: 16x2 HD44780 LCD driver in 4-bit mode with 4x4 keypad navigation
 */
#include <reg51.h>
#include "lcd16x2.h"

unsigned char scan_keypad_nav(void) {
    /* Read Port 0 for directional keys: 2 (Up), 8 (Down), 4 (Left), 6 (Right) */
    unsigned char key = P0 & 0x0F;
    if (key == 0x02) return 'U';
    if (key == 0x08) return 'D';
    if (key == 0x04) return 'L';
    if (key == 0x06) return 'R';
    return 0;
}

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++);
    }
}

void main(void) {
    unsigned char row = 0, col = 0;
    lcd_init();
    
    lcd_goto(0, 0);
    lcd_puts("IoT AT89S52 LCD");
    lcd_goto(1, 0);
    lcd_puts("Pos: (0, 0)");
    
    delay_ms(1000);
    lcd_clear();
    lcd_goto(row, col);
    lcd_data('*');
    
    while (1) {
        unsigned char nav = scan_keypad_nav();
        if (nav != 0) {
            lcd_goto(row, col);
            lcd_data(' '); /* Erase previous position */
            
            if (nav == 'U') row = 0;
            if (nav == 'D') row = 1;
            if (nav == 'L' && col > 0) col--;
            if (nav == 'R' && col < 15) col++;
            
            lcd_goto(row, col);
            lcd_data('*');
            delay_ms(200); /* Debounce delay */
        }
    }
}
