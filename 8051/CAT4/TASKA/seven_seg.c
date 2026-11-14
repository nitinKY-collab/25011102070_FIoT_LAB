#include <reg51.h>
#include "seven_seg.h"

/* Common Cathode 7-Segment lookup table for digits 0-9 (Port 0: a-g on P0.0-P0.6) */
static const unsigned char seg_code[10] = {
    0x3F, /* 0: a,b,c,d,e,f */
    0x06, /* 1: b,c */
    0x5B, /* 2: a,b,d,e,g */
    0x4F, /* 3: a,b,c,d,g */
    0x66, /* 4: b,c,f,g */
    0x6D, /* 5: a,c,d,f,g */
    0x7D, /* 6: a,c,d,e,f,g */
    0x07, /* 7: a,b,c */
    0x7F, /* 8: a,b,c,d,e,f,g */
    0x6F  /* 9: a,b,c,d,f,g */
};

void display_digit(unsigned char value) {
    if (value <= 9) {
        P0 = seg_code[value];
    } else {
        P0 = 0x79; /* 'E' for error */
    }
}
