#include <reg51.h>
#include "lcd16x2.h"

sbit LCD_RS = P2^0;
sbit LCD_EN = P2^1;

static void delay_lcd(unsigned int us) {
    unsigned int i; for (i = 0; i < us; i++);
}

static void send_nibble(unsigned char nibble) {
    P2 = (P2 & 0x0F) | (nibble & 0xF0);
    LCD_EN = 1;
    delay_lcd(2);
    LCD_EN = 0;
    delay_lcd(2);
}

void lcd_cmd(unsigned char cmd) {
    LCD_RS = 0;
    send_nibble(cmd);
    send_nibble(cmd << 4);
    delay_lcd(50);
}

void lcd_data(unsigned char dat) {
    LCD_RS = 1;
    send_nibble(dat);
    send_nibble(dat << 4);
    delay_lcd(50);
}

void lcd_init(void) {
    delay_lcd(2000); /* Wait for power-on */
    LCD_RS = 0;
    LCD_EN = 0;
    
    /* 4-bit initialization sequence */
    send_nibble(0x30); delay_lcd(500);
    send_nibble(0x30); delay_lcd(150);
    send_nibble(0x30); delay_lcd(150);
    send_nibble(0x20); delay_lcd(150); /* Set 4-bit interface */
    
    lcd_cmd(0x28); /* 4-bit mode, 2-line display, 5x8 font */
    lcd_cmd(0x0C); /* Display ON, Cursor OFF */
    lcd_cmd(0x06); /* Entry mode auto-increment */
    lcd_cmd(0x01); /* Clear display */
    delay_lcd(2000);
}

void lcd_clear(void) {
    lcd_cmd(0x01);
    delay_lcd(2000);
}

void lcd_goto(unsigned char row, unsigned char col) {
    unsigned char address = (row == 0) ? (0x80 + col) : (0xC0 + col);
    lcd_cmd(address);
}

void lcd_puts(const char *str) {
    while (*str) {
        lcd_data(*str++);
    }
}

void lcd_print_number(unsigned int num) {
    char buf[6];
    int idx = 0;
    if (num == 0) {
        lcd_data('0');
        return;
    }
    while (num > 0) {
        buf[idx++] = (num % 10) + '0';
        num /= 10;
    }
    while (idx > 0) {
        lcd_data(buf[--idx]);
    }
}
