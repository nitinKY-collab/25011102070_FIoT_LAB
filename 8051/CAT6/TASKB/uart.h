#ifndef UART_H
#define UART_H

void uart_init(void);
void uart_write(char c);
void uart_send_string(const char *str);
char uart_read(void);
unsigned char uart_available(void);

#endif
