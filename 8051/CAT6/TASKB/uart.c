#include <reg51.h>
#include "uart.h"

#define BUFFER_SIZE 32
static volatile char rx_buffer[BUFFER_SIZE];
static volatile unsigned char rx_head = 0;
static volatile unsigned char rx_tail = 0;

void uart_init(void) {
    /* Timer 1 Mode 2: 8-bit auto-reload */
    TMOD = (TMOD & 0x0F) | 0x20;
    /* Baud rate 9600 @ 11.0592 MHz: TH1 = 256 - (11059200 / (384 * 9600)) = 0xFD */
    TH1 = 0xFD;
    TL1 = 0xFD;
    TR1 = 1; /* Start Timer 1 */
    
    /* SCON: Mode 1 (8-bit UART), REN=1 (Enable Reception) */
    SCON = 0x50;
    
    ES = 1; /* Enable Serial Interrupt */
    EA = 1; /* Enable Global Interrupts */
}

void uart_isr(void) interrupt 4 {
    if (RI) {
        RI = 0;
        rx_buffer[rx_head] = SBUF;
        rx_head = (rx_head + 1) % BUFFER_SIZE;
    }
    if (TI) {
        TI = 0;
    }
}

void uart_write(char c) {
    SBUF = c;
    while (!TI);
    TI = 0;
}

void uart_send_string(const char *str) {
    while (*str) {
        uart_write(*str++);
    }
}

unsigned char uart_available(void) {
    return (rx_head != rx_tail);
}

char uart_read(void) {
    char c = 0;
    if (rx_head != rx_tail) {
        c = rx_buffer[rx_tail];
        rx_tail = (rx_tail + 1) % BUFFER_SIZE;
    }
    return c;
}
