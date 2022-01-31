#ifndef _PINS_H
#define _PINS_H

#include <stdint.h>

typedef struct pin_port
{
    volatile uint8_t* Px_BASE;
    uint8_t pin_number;
    uint8_t mmio_offset;
} pin_port_t;

#define PORT_BASE(x) ((x).Px_BASE + (x).mmio_offset)

const extern pin_port_t button_read_pins[];

const extern pin_port_t button_clear_pin;

#endif
