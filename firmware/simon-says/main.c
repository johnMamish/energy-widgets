#include "main.h"
#include "uart_buffer.h"
#include "pins.h"
#include "simon_says.h"

#define NUM_BUTTONS 4

/**
 * Returns '1' if any of the buttons were read as pressed.
 */
void read_button_states(uint8_t* dest,
                        afig0007_clear_state_t* button_states,
                        const pin_port_t* button_read_pins,
                        int num_buttons);

void clear_timed_out_buttons(afig0007_clear_state_t* button_clear_states,
                             const pin_port_t* button_clear_pin,
                             int num_buttons);

int main()
{
    ////////////////////////////////////////////////
    // SETUP HARDWARE
    ////////////////////////////////////////////////
    // disable watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // todo: make all pins pulldown for power saving (see 12.3.2)

    // todo: increase clock frequency?
    // notes: see CSCTL1 register and Figure 3-1: clock system bus diagram; MCLK and SMCLK both use
    // DCO  / 8 at startup; DCO is set to 8MHz at startup.

    ////////////////////////////////////////////////
    // enable uart:
    //  * Pin P2.0 is connected to eUSCI_A0 TXD. It's the "txd" header connecting to the programmer
    //    on the msp430 launchpad. Table 6-23 of the DATASHEET (not the user guide) tells how to
    //    configure the output; eUSCI_A0 is the "secondary function", so PxSEL<1:0> needs to be
    //    0b10.
    //  * use SMCLK (1MHz) for baud generation.
    P2SEL1 |= (1 << 0);
    UCA0CTLW0 |= (0b11 << 6);

    // set the baud rate to 19200
    UCA0MCTLW = (0x20 << 8) | (10 << 4) | (1 << 0);
    UCA0BRW     = 6;

    // enable the UART
    UCA0CTLW0 &= ~(1 << 0);

    ////////////////////////////////////////////////
    // enable Timer_A0
    // use SMCLK (1MHz), divide by 8, and run timer in continuous mode
    TA0CTL = (0b10 << 8) | (0b11 << 6) | (0b10 << 4);

    P1OUT = 0;
    P8OUT = 0;

    // make button read pins inputs with pullups
    for (int i = 0; i < (2 * NUM_BUTTONS); i++) {
        volatile uint8_t* const port_base = PORT_BASE(button_read_pins[i]);
        port_base[OFS_P1DIR] &= ~(1 << button_read_pins[i].pin_number);
        port_base[OFS_P1REN] |= (1 << button_read_pins[i].pin_number);
        port_base[OFS_P1OUT] |= (1 << button_read_pins[i].pin_number);
    }

    // make button write pins outputs, and initialize the output to 0
    PORT_BASE(button_clear_pin)[OFS_P1OUT] &= ~(1 << button_clear_pin.pin_number);
    PORT_BASE(button_clear_pin)[OFS_P1DIR] |=  (1 << button_clear_pin.pin_number);

    // enable pin p1.0 and p1.1 as outputs for toggling
    P1DIR |= (1 << 3) | (1 << 2) | (1 << 0) | (1 << 1);

    // User guide section 2.3.4 - "After a power cycle I/O pins are locked in high-impedance state
    // with input Schmitt triggers disabled until LOCKLPM5 is cleared by the user software"
    PM5CTL0 &= ~LOCKLPM5;

    ////////////////////////////////////////////////
    // initialize data structures
    //
    saeclib_circular_buffer_t uart_buffer_static =
        saeclib_circular_buffer_salloc(256, sizeof(uint8_t));
    uart_buffer = &uart_buffer_static;

    // This buffer holds timestamps describing the onset time of each button read
    saeclib_circular_buffer_t clear_pulse_buffer = saeclib_circular_buffer_salloc(64, sizeof(uint16_t));

    static uint8_t button_states[2 * NUM_BUTTONS] = { 0 };
    static afig0007_clear_state_t button_clear_states[2 * NUM_BUTTONS] = { 0 };

    // TODO: intermittent logic.
    // TODO: init random seed
    // idea: initial value in FRAM is {xx, xx, xx, ..., .game_lost = 1, xx, xx}.
    static simon_says_gamestate_t gs = {
        .lfsr_seed = 0, .lfsr = 0,
        .t_now = 0, .t_last_press = 0,
        .level_num = 0, .step_num = 0, .player_active = 0,
        .game_lost = 1,
        .output = 0, .output_high_time = 0
    };

    if (gs.game_lost) {
        simon_says_init(&gs, 0x1313);
    }

    char char_map[] = { '1', 'a', '2', 'b', '3', 'c', '4', 'd' };

    while (1) {
        // figure out which button was pressed
        read_button_states(button_states, button_clear_states, button_read_pins, NUM_BUTTONS);
        for (int i = 0; i < (2 * NUM_BUTTONS); i++)
            if (button_states[i]) saeclib_circular_buffer_pushone(uart_buffer, &char_map[i]);

        // clear pending buttons after a time delay
        clear_timed_out_buttons(button_clear_states, &button_clear_pin, NUM_BUTTONS);

        // update game state
        simon_says_advance_gamestate(&gs, 0, 1);

        // polled UART
        if ((UCA0IFG & (1 << 1)) && (!saeclib_circular_buffer_empty(uart_buffer))) {
            uint8_t ch;
            saeclib_circular_buffer_popone(uart_buffer, &ch);
            UCA0TXBUF = ch;
        }
    }
    while(1);

    return -1;
}

void read_button_states(uint8_t* button_states,
                        afig0007_clear_state_t* button_clear_states,
                        const pin_port_t* button_read_pins,
                        int num_buttons)
{
    const uint16_t read_time = TA0R;

    for (int i = 0; i < (2 * num_buttons); i++) {
        button_states[i] = 0;

        uint8_t port_state = PORT_BASE(button_read_pins[i])[OFS_P1IN];

        if (!(port_state & (1 << button_read_pins[i].pin_number)) &&
            !(button_clear_states[i].button_clear_pending)) {
            button_states[i] = 1;
            button_clear_states[i].button_clear_pending = 1;
            button_clear_states[i].timestamp = read_time;
        }
    }
}

static int_fast8_t button_clear_pending(const uint16_t t_now,
                                        const afig0007_clear_state_t* button_clear_state)
{
    const static uint16_t delay_ta0 = 10000 / 8;
    return (button_clear_state->button_clear_pending &&
            ((button_clear_state->timestamp - t_now) > delay_ta0));
}

void clear_timed_out_buttons(afig0007_clear_state_t* button_clear_states,
                             const pin_port_t* button_clear_pin,
                             int num_buttons)
{
    const uint16_t read_time = TA0R;

    int_fast8_t button_needs_clear = 0;
    for (int i = 0; i < (2 * num_buttons); i++) {
        if (button_clear_pending(read_time, &button_clear_states[i])) {
            button_needs_clear = 1;
            button_clear_states[i].button_clear_pending = 0;
        }
    }

    if (button_needs_clear) {
        // clear the buttons with a very short, blocking pulse.
        PORT_BASE(*button_clear_pin)[OFS_P1OUT] |= (1 << button_clear_pin->pin_number);
        for (volatile int tstart = TA0R; (TA0R - tstart) < (32 / 8););
        PORT_BASE(*button_clear_pin)[OFS_P1OUT] &= ~(1 << button_clear_pin->pin_number);
    }
}
