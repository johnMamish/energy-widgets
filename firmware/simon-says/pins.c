#include "msp430fr5994.h"
#include "pins.h"

const pin_port_t button_read_pins[] =
{
    {(uint8_t*)P4_BASE, 4, OFS_P2IN},     // button 0 down: P4.4
    {(uint8_t*)P5_BASE, 7, OFS_P1IN},     // button 0 up:   P5.7

    {(uint8_t*)P7_BASE, 3, OFS_P1IN},
    {(uint8_t*)P5_BASE, 3, OFS_P1IN},

    {(uint8_t*)P8_BASE, 3, OFS_P2IN},
    {(uint8_t*)P2_BASE, 5, OFS_P2IN},

    {(uint8_t*)P8_BASE, 1, OFS_P2IN},
    {(uint8_t*)P8_BASE, 2, OFS_P2IN},
};

const pin_port_t button_clear_pin = {(uint8_t*)P3_BASE, 6, OFS_P1IN};
