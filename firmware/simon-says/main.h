#ifndef _MAIN_H
#define _MAIN_H

#include "saeclib/src/saeclib_circular_buffer.h"
#include "msp430fr5994.h"
#include <stdint.h>

/**
 * This struct holds timestamp information needed to determine when to set the button clear
 * strobe.
 */
typedef struct afig0007_clear_state {
    uint8_t button_clear_pending;
    uint16_t timestamp;
} afig0007_clear_state_t;

#endif
