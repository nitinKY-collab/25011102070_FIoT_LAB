#include <reg51.h>
#include "timer.h"

void timer0_init_1ms(void) {
    /* Timer 0 Mode 1: 16-bit timer */
    TMOD = (TMOD & 0xF0) | 0x01;
    /* Preload for 1ms @ 11.0592 MHz: 65536 - 922 = 64614 = 0xFC66 */
    TH0 = 0xFC;
    TL0 = 0x66;
    ET0 = 1; /* Enable Timer 0 Interrupt */
    EA  = 1; /* Enable Global Interrupts */
    TR0 = 1; /* Start Timer 0 */
}
