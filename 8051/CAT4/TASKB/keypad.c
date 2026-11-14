#include <reg51.h>
#include "keypad.h"

/* Port 2: P2.0-P2.3 Rows (Outputs), P2.4-P2.7 Cols (Inputs with pull-ups) */
unsigned char keypad_scan(void) {
    unsigned char row, col;
    unsigned char row_mask[4] = {0xFE, 0xFD, 0xFB, 0xF7};
    unsigned char key_matrix[4][4] = {
        {1, 2, 3, 10},
        {4, 5, 6, 11},
        {7, 8, 9, 12},
        {14, 0, 15, 13}
    };
    
    for (row = 0; row < 4; row++) {
        P2 = row_mask[row] | 0xF0;
        col = P2 >> 4;
        if (col != 0x0F) {
            /* Debounce delay */
            unsigned int d; for (d = 0; d < 1200; d++);
            col = P2 >> 4;
            if (col != 0x0F) {
                if ((col & 0x01) == 0) return key_matrix[row][0];
                if ((col & 0x02) == 0) return key_matrix[row][1];
                if ((col & 0x04) == 0) return key_matrix[row][2];
                if ((col & 0x08) == 0) return key_matrix[row][3];
            }
        }
    }
    return 0xFF; /* No key pressed */
}
