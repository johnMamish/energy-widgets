/**
 * This app counts button presses
 *
 * It communicates over UART with a baud rate of 9600.
 * It responds to single-character commands sent over UART.
 * It recognizes 2 commands:
 *   - 'r': reads the existing button counts; will respond with an ascii message containing
 *          the accumulated button counts. The format should be self-explanatory. All numbers
 *          are given in hex.
 *   - 'c': clears the existing button counts;
 */

#include "main.h"
#include "uart_buffer.h"
#include "pins.h"

#define NUM_BUTTONS 4

static uint16_t user_histogram[NUM_BUTTONS] __attribute__((persistent)) = { 0 };

static void init_hardware();

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

/**
 * @param[in]     button_states    Array containing one byte for each direction of each button.
 *                                 the 2n-th element should be set when the nth button has
 *                                 been pressed, and the (2n + 1)th element should be set when the
 *                                 nth button has been released
 * @param[in,out] histogram        Histogram to update
 */
static void update_user_histogram(const uint8_t* button_states, uint16_t* histogram, int n);

static inline uint8_t uint8_to_hexdigit(uint8_t n)
{
    return (n < 10) ? (n + '0') : (n + ('a' - 10));
}

int main()
{
    init_hardware();

    ////////////////////////////////////////////////
    // initialize data structures
    //
    saeclib_u8_circular_buffer_t uart_buffer_static =
        saeclib_u8_circular_buffer_salloc(256);
    uart_buffer = &uart_buffer_static;

    static uint8_t button_states[2 * NUM_BUTTONS] = { 0 };
    static afig0007_clear_state_t button_clear_states[2 * NUM_BUTTONS] = { 0 };

    while (1) {
        ////////////////////////////////////////////////
        // update buttons
        // figure out which button was pressed
        read_button_states(button_states, button_clear_states, button_read_pins, NUM_BUTTONS);

        // clear pending buttons after a time delay
        clear_timed_out_buttons(button_clear_states, &button_clear_pin, NUM_BUTTONS);
        update_user_histogram(button_states, user_histogram, NUM_BUTTONS);

        ////////////////////////////////////////////////
        // handle polled UART
        if (UCA0IFG & (1 << 0)) {
            // UART has a character waiting for reception;
            uint8_t ch = UCA0RXBUF;

            // don't defer processing, app is simple enough that we can do it right here.
            static char response_buf[5 * NUM_BUTTONS + 8];
            switch (ch) {
                case 'r': {
                    for (int i = 0; i < NUM_BUTTONS; i++) {
                        uint16_t n = user_histogram[i];
                        for (int j = 3; j >= 0; j--) {
                            response_buf[5 * i + j] = uint8_to_hexdigit(n & 0x000f);
                            n >>= 4;
                        }
                        response_buf[5 * i + 4] = ' ';
                    }

                    response_buf[5 * NUM_BUTTONS] = '\r';
                    response_buf[5 * NUM_BUTTONS + 1] = '\n';
                    response_buf[5 * NUM_BUTTONS + 2] = '\0';

                    for (int i = 0; response_buf[i]; i++)
                        saeclib_u8_circular_buffer_pushone(uart_buffer, response_buf[i]);

                    break;
                }

                case 'c': {
                    for (int i = 0; i < NUM_BUTTONS; i++) {
                        user_histogram[i] = 0;
                    }

                    break;
                }

                default: {
                    break;
                }
            }
        }

        // polled UART
        if ((UCA0IFG & (1 << 1)) && (!saeclib_u8_circular_buffer_empty(uart_buffer))) {
            uint8_t ch;
            saeclib_u8_circular_buffer_popone(uart_buffer, &ch);
            UCA0TXBUF = ch;
        }
    }
    while(1);

    return -1;
}

static void init_hardware()
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
    //  * Pin P2.1 is connected to eUSC1_A0 RXD.
    //    on the msp430 launchpad. Table 6-23 of the DATASHEET (not the user guide) tells how to
    //    configure the output; eUSCI_A0 is the "secondary function", so PxSEL<1:0> needs to be
    //    0b10.
    //  * use SMCLK (1MHz) for baud generation.
    P2SEL1 |= (1 << 0);
    P2SEL1 |= (1 << 1);
    UCA0CTLW0 |= (0b11 << 6);      // use SMCLK for baud generation

    // set the baud rate to...
    // 9600
    // UCA0BRW     = 104;
    // 115200
    UCA0MCTLW = (0xd6U << 8) | (0 << 4) | (0 << 0);
    UCA0BRW = 8;

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

    ////////////////////////////////////////////////////////////////
    // PWM for audio generation
    // Sample rate is SMCLK / 8 = 12.5kHz
    // Timer config
    TB0CTL = ((0b00u    << 13) |      // Each TBxCLn latch loads independently.
              (0b00u    << 11) |      // Counter length is 16 bits
              (0b10u    <<  8) |      // Use SMCLK for timer
              (0b00u    <<  6) |      // Input divider is /1
              (0b01u    <<  4) |      // Timer counts in 'up' mode; rolls over at TB0CCR0
              (0b0u     <<  2) |      // Don't reset timer
              (0b0u     <<  1) |      // interrupt enable
              (0b0u     <<  0));      // interrupt pending flag

    // Channel 0 is responsible for resetting channel outputs
    TB0CCTL0 = ((0b00u  << 14) |      // Capture mode is "no capture"
                (0b10u  << 12) |      // Cap/Compare input select is din't care.
                (0b0u   << 11) |      // Synchronize capture source is don't care.
                (0b00u  <<  9) |      // TB0CL0 loads immediately when CCR0 is written.
                (0b0u   <<  8) |      // Compare mode
                (0b000u <<  5) |      // Output mode is don't care.
                (0b0u   <<  4) |      // Capture/Compare interrupt enable
                (0b0000u << 0));      // Bottom 4 bits are all don't care.
    TB0CCR0  = 0u;

    // Channel 3 is positive output.
    TB0CCTL3 = ((0b00u  << 14) |      // Capture mode is "no capture"
                (0b10u  << 12) |      // Cap/Compare input select is don't care.
                (0b0u   << 11) |      // Synchronize capture source is don't care.
                (0b01   <<  9) |      // TB0CL3 loads when the timer hits 0.
                (0b0    <<  8) |      // Compare mode
                (0b111u <<  5) |      // Output mode: Chan3 output is set when timer hits CCR0 &
                                      // cleared when timer hits CCR3.
                (0b0u   <<  4) |      // Capture/Compare interrupt disable
                (0b0000u << 0));      // Bottom 4 bits are don't care.
    TB0CCR3  = 0u;

    // Channel 4 is negative output.
    TB0CCTL4 = ((0b00u  << 14) |      // Capture mode is "no capture"
                (0b10u  << 12) |      // Cap/Compare input select is don't care.
                (0b0u   << 11) |      // Synchronize capture source is don't care.
                (0b01   <<  9) |      // TB0CL3 loads when the timer hits 0.
                (0b0    <<  8) |      // Compare mode
                (0b011u <<  5) |      // Output mode: "set/reset". Channel is set when timer hits
                                      // CCR4 and cleared when timer hits CCR0.
                (0b0u   <<  4) |      // Capture/Compare interrupt disable
                (0b0000u << 0));      // Bottom 4 bits are don't care.
    TB0CCR4  = 0u;

    // Use Pin P3.4 for timer B channel TB0.3; see table 9-28 of the datasheet.
    P3SEL0 |=  (1 << 4);
    P3SEL1 &= ~(1 << 4);
    P3DIR  |=  (1 << 4);

    // Use Pin P3.5 for timer B channel TB0.4; see table 9-28 of the datasheet.
    P3SEL0 |=  (1 << 5);
    P3SEL1 &= ~(1 << 5);
    P3DIR  |=  (1 << 5);

    DMA1SA   = 0;
    DMA1DA   = 0;
    DMA1SZ   = 0;
    DMACTL0 &= ~(0b11111u << 8); DMACTL0 |= DMA1TSEL__TB0CCR0;
    DMA1CTL  = ((0b000u  << 12) |     // DMA xfer mode: single xfer
                (0b00u   << 10) |     // Don't increment destination.
                (0b11u   <<  8) |     // Do    increment source.
                (0b0u    <<  7) |     // Destination is a word, not a byte.
                (0b1u    <<  6) |     // Source is a byte, not a word.
                (0b1u    <<  5) |     // level sensitive DMA trigger
                (0b0u    <<  4) |     // DMA Enable
                (0b0000u <<  0));     // Bottom 4 bits are flags / don't care.

    // User guide section 2.3.4 - "After a power cycle I/O pins are locked in high-impedance state
    // with input Schmitt triggers disabled until LOCKLPM5 is cleared by the user software"
    PM5CTL0 &= ~LOCKLPM5;
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
    const static uint16_t delay_ta0 = 5000 / 8;
    return (button_clear_state->button_clear_pending &&
            ((t_now - button_clear_state->timestamp) > delay_ta0));
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


static void update_user_histogram(const uint8_t* button_states, uint16_t* histogram, int n)
{
    for (int i = 0; i < n; i++) {
        if (button_states[2 * i]) histogram[i]++;
    }
}
