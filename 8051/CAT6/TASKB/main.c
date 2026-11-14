/*
 * File: main.c
 * Target: AT89S52 (Continuous Evaluation 6)
 * Description: Interrupt-driven UART driver (9600-8-N-1) with PC PuTTY interactive echo
 */
#include <reg51.h>
#include "uart.h"

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++);
    }
}

void main(void) {
    uart_init();
    
    /* Transmit initial welcome banner */
    uart_send_string("Hello from 8051\r\n");
    uart_send_string("UART Echo Service Initialized (9600-8-N-1)\r\n");
    
    while (1) {
        if (uart_available()) {
            char c = uart_read();
            uart_write(c); /* Echo back received character */
        }
    }
}
