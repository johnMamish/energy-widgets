#if 1
#include "msp430fr5994.h"

#include <stdint.h>

#define NUM_BUTTONS 4

volatile uint8_t* const button_read_ports[] =
{
    // button 0
    (uint8_t*)P4_BASE,         // down: P4.4
    (uint8_t*)P5_BASE,         // up:   P5.7

    // button 1
    (uint8_t*)P7_BASE,         // down: P7.3
    (uint8_t*)P5_BASE,         // up:   P5.3

    // button 2
    (uint8_t*)P8_BASE,         // down: P8.3
    (uint8_t*)P2_BASE,         // up:   P2.5

    // button 3
    (uint8_t*)P8_BASE,         // down: P8.1
    (uint8_t*)P8_BASE          // up:   P8.2
};

const uint8_t button_read_pins[] =
{
    // button 0
    4,               // down
    7,               // up

    // button 1
    3,               // down
    3,               // up

    // button 2
    3,
    5,

    // button 3
    1,
    2
};

// if the corresponding port is odd-numbered, the corresponding value in this array should be 0, if
// the port is even-numbered, the corresponding value in this array should be 1. This is to account
// for the fact that P1 and P2's mmio controls are "interlaced", and so are P3 & P4's, etc.
const uint8_t button_read_mmio_offsets[] =
{
    OFS_P2IN,
    OFS_P1IN,

    OFS_P1IN,
    OFS_P1IN,

    OFS_P2IN,
    OFS_P2IN,

    OFS_P2IN,
    OFS_P2IN
};

volatile uint8_t* const button_clear_ports[] =
{
    // button 0
    (uint8_t*)P3_BASE,         // down: P3.6
    (uint8_t*)P3_BASE,         // up:   P3.7

    // button 1
    (uint8_t*)P3_BASE,         // down: P3.4
    (uint8_t*)P3_BASE,         // up:   P3.5

    // button 2
    (uint8_t*)P4_BASE,         // down: P4.3
    (uint8_t*)P2_BASE,         // up:   P2.6

    // button 3
    (uint8_t*)P4_BASE,         // down: P4.1
    (uint8_t*)P4_BASE          // up:   P4.2
};

const uint8_t button_clear_pins[] =
{
    // button 0
    6,               // down
    7,               // up

    // button 1
    4,               // down
    5,               // up

    // button 2
    3,               // down
    6,               // up

    // button 3
    1,               // down
    2                // up
};

const uint8_t button_clear_mmio_offsets[] =
{
    OFS_P1IN,
    OFS_P1IN,

    OFS_P1IN,
    OFS_P1IN,

    OFS_P2IN,
    OFS_P2IN,

    OFS_P2IN,
    OFS_P2IN,
};

int main()
{
    // disable watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // todo: make all pins pulldown for power saving (see 12.3.2)


    // todo: increase clock frequency?
    // notes: see CSCTL1 register and Figure 3-1: clock system bus diagram; MCLK and SMCLK both use
    // DCO  / 8 at startup; DCO is set to 8MHz at startup.

    // enable uart:
    //  * Pin P2.0 is connected to eUSCI_A0 TXD. It's the "txd" header connecting to the programmer
    //    on the msp430 launchpad. Table 6-23 of the DATASHEET tells how to configure the output;
    //    eUSCI_A0 is the "secondary function", so PxSEL<1:0> needs to be 0b10.
    //  * use SMCLK (1MHz) for baud generation.
    P2SEL1 |= (1 << 0);
    UCA0CTLW0 |= (0b11 << 6);

    // set the baud rate
    UCA0MCTLW = (0x20 << 8) | (10 << 4) | (1 << 0);
    UCA0BRW     = 6;

    // enable the UART
    UCA0CTLW0 &= ~(1 << 0);

    // make button read pins inputs
    for (int i = 0; i < (2 * NUM_BUTTONS); i++)
        *(button_read_ports[i] + button_read_mmio_offsets[i] + OFS_P1DIR) &= ~(1 << button_read_pins[i]);

    // make button write pins outputs, and initialize the output to 0
    for (int i = 0; i < (2 * NUM_BUTTONS); i++) {
        *(button_clear_ports[i] + button_clear_mmio_offsets[i] + OFS_P1OUT) &= ~(1 << button_clear_pins[i]);
        *(button_clear_ports[i] + button_clear_mmio_offsets[i] + OFS_P1DIR) |= (1 << button_clear_pins[i]);
    }

    // enable pin p1.0 and p1.1 as outputs for toggling
    P1OUT = 0;
    P8OUT = 0;
    //P1DIR |= (1 << 4) | (1 << 2) | (1 << 1) | (1 << 0);
    P1DIR |= (1 << 3) | (1 << 2) | (1 << 0) | (1 << 1);
    uint8_t output_indicate_pin_nums[] = { 0, 0, 2, 2, 3, 3, 1, 1 };
    int char_map[] = { '1', -1, '2', -1, '3', -1, '4', -1 };
    int char_to_send = -2;

    //
    PM5CTL0 &= ~LOCKLPM5;

    // figure out which button was pressed
    uint8_t button_states[] = { 0, 0, 0, 0 };
    for (int i = 0; i < (2 * NUM_BUTTONS); i++) {
        if (!(*(button_read_ports[i] + button_read_mmio_offsets[i] + OFS_P1IN) & (1 << button_read_pins[i]))) {
            button_states[i] = 1;
            P1OUT |= (1 << output_indicate_pin_nums[i]);
            char_to_send = char_map[i];
        }
        *(button_clear_ports[i] + button_clear_mmio_offsets[i] + OFS_P1OUT) |= (1 << button_clear_pins[i]);
    }

    if (char_to_send > 0) {
        UCA0TXBUF = char_to_send;
    }

    if (char_to_send == -2) {
        P1OUT |= (1 << 0);
    }
    while(1);

    return -1;
}

#else

#include "msp430fr5994.h"

#include <stdint.h>

volatile uint8_t* const button_read_ports[] =
{
    // button 1
    (uint8_t*)P3_BASE,         // down
    (uint8_t*)P3_BASE,         // up

    // button 2
    (uint8_t*)P3_BASE,         // down
    (uint8_t*)P3_BASE          // up
};

const uint8_t button_read_pins[] =
{
    // button 1
    7,               // down
    6,               // up

    // button 2
    5,               // down
    4                // up
};

// if the corresponding port is odd-numbered, the corresponding value in this array should be 0, if
// the port is even-numbered, the corresponding value in this array should be 1. This is to account
// for the fact that P1 and P2's mmio controls are "interlaced", and so are P3 & P4's, etc.
const uint8_t button_read_mmio_offsets[] =
{
    OFS_P1IN,
    OFS_P1IN,
    OFS_P1IN,
    OFS_P1IN
};

volatile uint8_t* const button_clear_ports[] =
{
    // button 1
    (uint8_t*)P8_BASE,         // down
    (uint8_t*)P8_BASE,         // up

    // button 2
    (uint8_t*)P8_BASE,         // down
    (uint8_t*)P8_BASE          // up
};

const uint8_t button_clear_pins[] =
{
    // button 1
    3,               // down
    2,               // up

    // button 2
    1,               // down
    0,               // up
};

const uint8_t button_clear_mmio_offsets[] =
{
    OFS_P2IN,
    OFS_P2IN,
    OFS_P2IN,
    OFS_P2IN
};

int main()
{
    // disable watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // todo: make all pins pulldown for power saving (see 12.3.2)


    // todo: increase clock frequency?


    // make button read pins inputs
    for (int i = 0; i < 4; i++)
        *(button_read_ports[i] + button_read_mmio_offsets[i] + OFS_P1DIR) &= ~(1 << button_read_pins[i]);

    // make button write pins outputs, and initialize the output to 0
    for (int i = 0; i < 4; i++) {
        *(button_clear_ports[i] + button_clear_mmio_offsets[i] + OFS_P1OUT) &= ~(1 << button_clear_pins[i]);
        *(button_clear_ports[i] + button_clear_mmio_offsets[i] + OFS_P1DIR) |= (1 << button_clear_pins[i]);
    }

    // enable pin p1.0 and p1.1 as outputs for toggling
    P1OUT = 0;
    P8OUT = 0;
    P1DIR |= (1 << 4) | (1 << 2) | (1 << 1) | (1 << 0);
    uint8_t output_indicate_pin_nums[] = { 0, 2, 1, 4 };

    //
    PM5CTL0 &= ~LOCKLPM5;

    // figure out which button was pressed
    uint8_t button_states[] = { 0, 0, 0, 0 };
    for (int i = 0; i < 4; i++) {
        if (!(*(button_read_ports[i] + button_read_mmio_offsets[i] + OFS_P1IN) & (1 << button_read_pins[i]))) {
            button_states[i] = 1;
            P1OUT |= (1 << output_indicate_pin_nums[i]);
        }
        *(button_clear_ports[i] + button_clear_mmio_offsets[i] + OFS_P1OUT) |= (1 << button_clear_pins[i]);
    }

    while(1) {
        //P1OUT ^= (1 << 1);
        for (volatile int i = 0; i < 10000; i++);
    }

    return -1;
}

#endif
