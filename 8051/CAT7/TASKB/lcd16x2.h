#ifndef LCD16X2_H
#define LCD16X2_H

void lcd_init(void);
void lcd_cmd(unsigned char cmd);
void lcd_data(unsigned char dat);
void lcd_puts(const char *str);
void lcd_goto(unsigned char row, unsigned char col);
void lcd_clear(void);
void lcd_print_number(unsigned int num);

#endif
