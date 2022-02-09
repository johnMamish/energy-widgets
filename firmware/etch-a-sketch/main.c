#include "msp430fr5994.h"

#include <stdint.h>
#include <string.h>

/**
 * TODOs:
 *   - figure out / confirm motor sample timing
 *   - screen DMA
 */

// fixed-point integer type
typedef uint32_t q24_8_t;
typedef int32_t q23_8_t;

// Gain of MAX9938 current sense
// 25 * 256 = 6400
const q24_8_t ISENSEGAIN_Q24_8 = 6400;

// Resistance of current sense resistor on MAX9938
// 3.33... * 256 = 853.333..
const q24_8_t RSENSE_Q24_8 = 853;

// multiplication factor of resistor divider on output of harvester
// (150 / (150 + 1000)) * 256 = 33.39
const q24_8_t RDIV_FACTOR_Q24_8 = 33;

// source resistance: 260 ohms for motor
const q24_8_t MOTOR_RWIND_Q24_8 = 66560;

// 2 * 256 = 512
const q24_8_t VCC_Q24_8 = 512;

/**
 * Convenience function that all of the hardware init functions are being dumped into
 */
static void init_hardware();

#if 0
static uint8_t digit_to_hexchar(int digit)
{
    return (digit >= 10) ? ((digit - 10) + 'a') : (digit + '0');
}

static void u16_to_hex(uint16_t val, char* buf)
{
    for (int i = 3; i >= 0; i--) {
        buf[i] = digit_to_hexchar(val & 0x000f);
        val >>= 4;
    }
}

static void puts_blocking(const char* s)
{
    for (int i = 0; s[i]; i++) { UCA0TXBUF = s[i]; while(UCA0STATW & (1 << 0)); }
}

static q24_8_t calculate_voc(uint16_t adc_isense, uint16_t adc_vsense, q24_8_t rwind)
{
    q24_8_t Ih = ((adc_isense * VCC_Q24_8) / ((RSENSE_Q24_8 * ISENSEGAIN_Q24_8) >> 8));
    q24_8_t Vh = ((adc_vsense * VCC_Q24_8) / (RDIV_FACTOR_Q24_8));
    return (Vh + ((Ih * rwind) >> 8));
}
#endif

static inline q23_8_t iabs_q23_8(q23_8_t n)
{
    return ((n < 0) ? -n : n);
}

/**
 * adc_results[0,2] are current for
 * adc_results[1,3] are voltage
 */
static volatile uint16_t adc_results[4];
static volatile uint8_t adc_ready = 0;

__attribute__((interrupt(ADC12_B_VECTOR)))
void adc12_sample_interrupt()
{
    // We expect that adc12 only ever runs in autoscan mode, it should start at mem location 0 and
    // end at mem location 3, so we check for the 'mem location 3 done' interrupt
    if (ADC12IFGR0 & (1 << 3)) {
        adc_results[0] = ADC12MEM0;
        adc_results[1] = ADC12MEM1;
        adc_results[2] = ADC12MEM2;
        adc_results[3] = ADC12MEM3;
        adc_ready = 1;
        ADC12CTL0 &= ~(1 << 1);
        ADC12CTL0 |= (1 << 1);
    } else {
        // unknown interrupt
        while(1);
    }
}

#define SCREEN_WIDTH (144u)
#define SCREEN_HEIGHT (168u)

#define LINE_PADDING_BYTES 2
#define BYTES_PER_LINE (SCREEN_WIDTH / 8)
#define MAX_LINES_PER_FRAME 24
#define POINTER_SPRITE_HEIGHT 3
#define DMA_BUFFER_SIZE (LINE_PADDING_BYTES + (BYTES_PER_LINE * (MAX_LINES_PER_FRAME + POINTER_SPRITE_HEIGHT)))

static uint8_t screen_buffers[2][DMA_BUFFER_SIZE];

static uint8_t persistent_bitmap[SCREEN_HEIGHT][SCREEN_WIDTH / 8] __attribute__ ((persistent)) = {
    0
};

void clear_screen(uint8_t persistent_bitmap[SCREEN_HEIGHT][SCREEN_WIDTH / 8]);

/**
 *
 */
uint16_t update_state(uint8_t persistent_bitmap[SCREEN_HEIGHT][SCREEN_WIDTH / 8],
                      q23_8_t* point,
                      q23_8_t* dpoint,
                      uint8_t* dma_buffer);

int main()
{
    init_hardware();

    // clear persistent screen
    memset(persistent_bitmap, 0xff, sizeof(persistent_bitmap));

    // enable interrupts
    //_EINT();

    TA0CTL |= (1 << 2);
    ADC12CTL0 |= (1 << 1);
    uint16_t tnow = TA0R / 128;
    uint8_t front_buffer = 0;

    // blocking code to clear screen
    P8OUT |= (1 << 3);
    UCB1TXBUF = (1 << 2);
    while (!(UCB1IFG & (1 << 1)));
    UCB1TXBUF = 0;
    while (!(UCB1IFG & (1 << 1)));
    for (uint16_t tstart = TA0R; (TA0R - tstart) < 2; );
    P8OUT &= ~(1 << 3);

    q23_8_t point[2] = {((SCREEN_WIDTH + 14)  / 2) * 256, ((SCREEN_HEIGHT - 8) / 2) * 256};
    q23_8_t dpoint[2] = {200 * 3, 256};
    while(1) {
        // polling wait until frame time (30Hz) is passed
        while (((TA0R / 128) - tnow) < 33);
        tnow = TA0R / 128;

        ////////////////////////////////////////////////
        // process data; fill buffer
        // fill command and vcom toggle
        uint16_t buffer_len = update_state(persistent_bitmap, point, dpoint, &screen_buffers[front_buffer][0]);

        // set toggling vcom
        screen_buffers[front_buffer][0] |= (front_buffer << 1);

        // move cursor
        point[0] += dpoint[0]; point[1] += dpoint[1];

        #if 1
        if (point[0] < 0) {
            point[0] = -point[0];
            dpoint[0] = -dpoint[0];
        } else if (point[0] > (SCREEN_WIDTH << 8)) {
            point[0] = (SCREEN_WIDTH << 8) - (point[0] - (SCREEN_WIDTH << 8));
            dpoint[0] = -dpoint[0];
        }

        if (point[1] < 0) {
            point[1] = -point[1];
            dpoint[1] = -dpoint[1];
        } else if (point[1] > (SCREEN_HEIGHT << 8)) {
            point[1] = (SCREEN_HEIGHT << 8) - (point[1] - (SCREEN_HEIGHT << 8));
            dpoint[1] = -dpoint[1];
        }
        #endif

        // poll on dma xfer being finished.
        while ((DMA3CTL & (1 << 4)));

        // toggle chip sel low and back high;
        // needs to be low for a minimum of 6 microseconds. TA0 ticks once every 128us
        P8OUT &= ~(1 << 3);
        for (uint16_t tstart = TA0R; (TA0R - tstart) < 2; );
        P8OUT |= (1 << 3);

        // start new DMA xfer
        DMA3SA = (intptr_t)(&screen_buffers[front_buffer][0]);
        DMA3DA = (intptr_t)(&UCB1TXBUF);
        DMA3SZ = buffer_len;
        DMA3CTL |= (1 << 4);

        front_buffer++;
        if (front_buffer == 2) front_buffer = 0;

#if 0
        _DINT();
        uint16_t adcnow[4] = {adc_results[0], adc_results[1], adc_results[2], adc_results[3]};
        _EINT();
        uint16_t voc_motor = (uint16_t)calculate_voc(adcnow[0], adcnow[1], MOTOR_RWIND_Q24_8);
        uint16_t voc_shaker = (uint16_t)calculate_voc(adcnow[2], adcnow[3], MOTOR_RWIND_Q24_8);
#endif
    }
    return -1;
}

static void init_hardware()
{
    // disable watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Unlock I/O configurations
    PM5CTL0 &= ~LOCKLPM5;

    // todo: make all pins pulldown for power saving (see 12.3.2)

    // leds P1.0 and P1.1 for debug
    P1OUT = 0;
    P1DIR |= (1 << 1) | (1 << 0);

    // Change SMCLK to be 8MHz / 1 instead of 8MHz / 8. Clock control registers need to be unlocked
    // by writing a key value to CSCTL0 first.
    CSCTL0  = (0xa5u << 8);
    CSCTL3 &= ~(0b111u << 4); CSCTL3 |= (0b000u << 4);

    // By default the CPU core clock (MCLK) is 8MHz / 8. We can keep it that way, or we can divide
    // it even further to save power on DMA and CPU (maybe ??)
    CSCTL3 &= ~(0b111u << 0); CSCTL3 |= (0x3u << 0);

    // make sure that the MCLKREQEN bit is set (should be set anyways after BOR)
    CSCTL6 |= (1 << 1);

    // enable uart:
    //  * Pin P2.0 is connected to eUSCI_A0 TXD. It's the "txd" header connecting to the programmer
    //    on the msp430 launchpad. Table 6-23 of the DATASHEET tells how to configure the output;
    //    eUSCI_A0 is the "secondary function", so PxSEL<1:0> needs to be 0b10.
    //  * use SMCLK (8MHz) for baud generation.
    P2SEL1 |= (1 << 0);
    UCA0CTLW0 |= (0b11 << 6);

    // set the baud rate
    //UCA0MCTLW = (0x20 << 8) | (10 << 4) | (1 << 0);
    //UCA0BRW     = 6 * 8;
    UCA0MCTLW = (0x55 << 8) | (5 << 4) | (1 << 0);
    UCA0BRW = 4;

    // enable the UART
    UCA0CTLW0 &= ~(1 << 0);

    ////////////////////////////////////////////////////////////////
    // Set up a timer for general purpose timekeeping.
    // TA0 runs at 125kHz
    TA0CTL = ((0b10u    <<  8) |      // Use SMCLK for timer
              (0b11u    <<  6) |      // Input divider is /8
              (0b10u    <<  4) |      // Timer counts in continuous mode; rolls over from 0xffff to 0x0000.
              (0b0u     <<  2) |      // Don't reset timer
              (0b0u     <<  1) |      // interrupt enable
              (0b0u     <<  0));      // interrupt pending flag
    TA0EX0 = (0b111 << 0);            // Divide by 8 again for a frequency of 125kHz.

    ////////////////////////////////////////////////////////////////
    // Use TA1 CCR1 to trigger the ADC at 1kHz
    // It will be clocked at 125kHz with a period of 125.
    TA1CTL = ((0b10u    <<  8) |      // Use SMCLK for timer
              (0b11u    <<  6) |      // Input divider is /8
              (0b01u    <<  4) |      // Timer counts in 'up mode', it goes from 0 to TA1CCR0
              (0b0u     <<  2) |      // Don't reset timer
              (0b0u     <<  1) |      // interrupt disabled
              (0b0u     <<  0));      // interrupt pending flag
    TA1EX0 = (0b111 << 0);            // Divide by 8 again for a frequency of 125kHz.

    TA1CCTL0 = ((0b00u  << 14) |      // Capture mode is "no capture"
                (0b10u  << 12) |      // Cap/Compare input select is don't care (but make it GND)
                (0b0u   << 11) |      // Synchronize capture source is don't care.
                (0b00u  <<  9) |      // TA1CL0 loads immediately when CCR0 is written.
                (0b0u   <<  8) |      // Compare mode
                (0b000u <<  5) |      // Output mode is don't care.
                (0b0u   <<  4) |      // Capture/Compare interrupt enable
                (0b0000u << 0));      // Bottom 4 bits are all don't care.
    TA1CCR0 = 125;

    TA1CCTL1 = ((0b00u  << 14) |      // Capture mode is "no capture"
                (0b10u  << 12) |      // Cap/Compare input select is don't care (but make it GND)
                (0b0u   << 11) |      // Synchronize capture source is don't care.
                (0b00u  <<  9) |      // TA1CL3 loads immediately when CCR0 is written.
                (0b0u   <<  8) |      // Compare mode
                (0b011u <<  5) |      // Output mode is 'set/reset'
                (0b0u   <<  4) |      // Capture/Compare interrupt enable
                (0b0000u << 0));      // Bottom 4 bits are all don't care.
    TA1CCR1 = (125 / 2);

    // Pin P1.2 is connected to TA1.1
    P1SEL0 |=  (1 << 2);
    P1SEL1 &= ~(1 << 2);
    P1DIR  |=  (1 << 2);

    ////////////////////////////////////////////////////////////////
    // Setup DMA channel to copy from SRAM to SPI
    // DMA channel 1 loads data from SRAM into SPI TXBUF (UCB1TXBUF).
    // It should be triggered by UCB1TX
    DMA3SA  = 0;
    DMA3DA  = 0;
    DMA3SZ  = 0;
    DMACTL1 &= ~(0b11111u << 8); DMACTL1 |= DMA3TSEL__UCB1TXIFG;
    DMA3CTL = ((0b000u  << 12) |     // DMA xfer mode: single xfer
               (0b00u   << 10) |     // Don't increment destination.
               (0b11u   <<  8) |     // Do    increment source.
               (0b1u    <<  7) |     // Destination is a byte, not a word
               (0b1u    <<  6) |     // Source is a byte, not a word.
               (0b1u    <<  5) |     // level sensitive DMA trigger
               (0b0u    <<  4) |     // DMA Enable
               (0b0000u <<  0));     // Bottom 4 bits are flags / don't care.

    ////////////////////////////////////////////////////////////////
    // Set up ADC to sample channels A12 - A15 (pins p3.0-3)
    // The sample rate over 9 (3 * 3) pins should be ~1kHz. The
    P3SEL0 |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
    P3SEL1 |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
    ADC12CTL0 = ((0b0000 << 12) |     // ADC samp/hold time 1 is 4 ADC clocks
                 (0b0000 << 8)  |     // ADC samp/hold time 2 is 4 ADC clocks
                 (0b1    << 7)  |     // ADC12MSC
                 (0b1    << 4)  |     // Turn ADC on
                 (0b0    << 1)  |     // Don't enable or start conversions yet
                 (0b0    << 0));

    ADC12CTL1 = ((0b10   << 13) |     // Pre-divide by 32 for an input clock of 250kHz
                 (0b100  << 10) |     // SHSx is TA1 CCR1
                 (0b1    << 9)  |     // Sample period is automatically controlled by the sample timer
                 (0b0    << 8)  |     // Invert sample/hold signal; don't care
                 (0b001  << 5)  |     // Divide by /2 extra
                 (0b11   << 3)  |     // SMCLK source: 8MHz
                 (0b01   << 1));      // single-channel, single-conversion
                                      // bit 0 is a read-only status bit.
    ADC12CTL2 = ((0b00   << 4)  |     // 8-bit resolution
                 (0b0    << 3)  |     // unsigned read-back
                 (0b00   << 1)  |     // bits 2-1 are don't care read-only
                 (0b1    << 0));      // enable low-power mode; clk is 125kHz, < 1/4th of 5.4MHz max

    // memory location 0 should hold results from sampling channel A12
    ADC12MCTL0 = (0b0 << 7) | (12 << 0);
    ADC12MCTL1 = (0b0 << 7) | (13 << 0);
    ADC12MCTL2 = (0b0 << 7) | (14 << 0);
    ADC12MCTL3 = (0b1 << 7) | (15 << 0);

    ADC12IER0 = (1 << 3);

    ////////////////////////////////////////////////////////////////
    // UCB1
    // UCB1 is set up as a SPI controller for the display

    // P5.2 and P5.0 are assigned to UCB1
    P5SEL0 |= (1 << 2) | (1 << 0);

    // P8.3 is a GPIO controlling the chip select line
    P8OUT &= ~(1 << 3);
    P8DIR |= (1 << 3);

    UCB1CTLW0 = ((0b1 << 15) |        // data is sampled on rising edge
                 (0b0 << 14) |        // clock's idle state is low
                 (0b0 << 13) |        // 1: MSB first
                 (0b0 << 12) |        // 0: 8-bit data
                 (0b1 << 11) |        // 1: SPI controller mode
                 (0b00 << 9) |        // 00: 3-pin spi mode
                 (0b1 <<  8) |        // synchronous mode
                 (0b10 << 6) |        // use SMCLK
                 (0b0  << 0));        // don't software reset; we want to use the hw

    UCB1BRW = 8;                      // serial clock = SMCLK/8 = 1MHz
}

/**
 * code for this copied from wikipedia
 */
static int32_t isqrt(int32_t n)
{
    int32_t x0 = n / 2;

    // Sanity check
    if ( x0 != 0 ) {
        int32_t x1 = ( x0 + n / x0 ) / 2;	// Update

        while (x1 < x0) {
            x0 = x1;
            x1 = (x0 + (n / x0)) / 2;
        }

        return x0;
    }
    else {
        return n;
    }
}

static q23_8_t length_q23_8(const q23_8_t* dp)
{
    int32_t a2 = ((int32_t)dp[0]) * ((int32_t)dp[0]);
    int32_t b2 = ((int32_t)dp[1]) * ((int32_t)dp[1]);
    return isqrt(a2 + b2);
}

uint16_t update_state(uint8_t persistent_bitmap[SCREEN_HEIGHT][SCREEN_WIDTH / 8],
                      q23_8_t* point,
                      q23_8_t* dpoint,
                      uint8_t* dma_buffer)
{
    uint8_t n_updated_lines = 0;
    static uint8_t updated_screen_lines[32];

    // simple and inaccurate fixed-point line drawing algorithm; dumb. room for optimization.
    q23_8_t length = length_q23_8(dpoint);
    q23_8_t normalized_dpoint[2] = {(dpoint[0] * 256) / length,
        (dpoint[1] * 256) / length};

    q23_8_t q[2] = {point[0], point[1]};
    for (int i = 0; i < (length / 256); i++) {
        // set pixel at point
        // note - black pixels are '0', white pixels are '1'
        uint8_t x = q[0] / 256;
        uint8_t y = q[1] / 256;
        persistent_bitmap[y][x / 8] &= ~(1 << (x % 8));

        // do a linear search through the lines we've already logged and decide not to log this line
        // if we've already logged it.
        int f = 0;
        for (int i = (n_updated_lines - 1); i >= 0; i--) {
            if (updated_screen_lines[i] == y) {
                f = 1;
                break;
            }
        }

        if (!f)
            updated_screen_lines[n_updated_lines++] = y;

        // move point in direction
        q[0] += normalized_dpoint[0];
        q[1] += normalized_dpoint[1];
    }

    uint8_t x = q[0] / 256;
    uint8_t y = q[1] / 256;
    persistent_bitmap[y][x / 8] &= ~(1 << (x % 8));
    updated_screen_lines[n_updated_lines++] = y;

    // TODO: caller needs to make sure to toggle vcom bit
    dma_buffer[0] = (1 << 0);
    uint16_t dma_idx = 1;
    for (int i = 0; i < n_updated_lines; i++) {
        int l = updated_screen_lines[i];

        // setup dma
        // line number
        dma_buffer[dma_idx] = l;

        // line data
        memcpy(&dma_buffer[dma_idx + 1], &persistent_bitmap[l][0], (SCREEN_WIDTH / 8));

        // trailer
        dma_buffer[dma_idx + 1 + (SCREEN_WIDTH / 8)] = 0x00;

        dma_idx += 1 + (SCREEN_WIDTH / 8) + 1;
    }
    dma_buffer[dma_idx++] = 0x00;

    // TODO: add cursor to dma buffer.

    // return length of buffer to send with DMA.
    return dma_idx;
}
