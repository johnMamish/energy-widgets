#include "msp430fr5994.h"

#include <stdint.h>

#include "audio_samples.h"
#include "game.h"

// fixed-point integer type
typedef uint32_t q24_8_t;

// Gain of MAX9938 current sense
// 25 * 256 = 6400
const q24_8_t ISENSEGAIN_Q24_8 = 6400;

// Resistance of current sense resistor on MAX9938
// 3.33... * 256 = 853.333..
const q24_8_t RSENSE_Q24_8 = 853;

// multiplication factor of resistor divider on output of harvester
// (150 / (150 + 1000)) * 256 = 33.39
const q24_8_t RDIV_FACTOR_Q24_8 = 33;

// source resistances: 260 and 240 for motor and 'faraday flashlight' respectively
const q24_8_t MOTOR_RWIND_Q24_8 = 66560;
const q24_8_t SHAKER_RWIND_Q24_8 = 61440;

// 2 * 256 = 512
const q24_8_t VCC_Q24_8 = 512;

/**
 * Convenience function that all of the hardware init functions are being dumped into
 */
static void init_hardware();
static void start_audio_dma(const uint8_t* src, int32_t len);


const uint8_t* action_sounds[] =
{
    __assets_bop_1_wav, __assets_bop_2_wav, __assets_bop_3_wav, __assets_bop_4_wav,
    __assets_shake_1_wav, __assets_shake_2_wav, __assets_shake_3_wav, __assets_shake_4_wav,
    __assets_twist_1_wav, __assets_twist_2_wav, __assets_twist_3_wav, __assets_twist_4_wav
};

const uint8_t* it_sounds[] =
{
    __assets_it_1_wav, __assets_it_2_wav, __assets_it_3_wav, __assets_it_4_wav
};

static void advance_lfsr(uint32_t* num)
{
    int bit = *num & 1;
    *num >>= 1;
    if(bit) {
        *num ^= ((1UL << 31) | (1UL << 29) | (1UL << 25) | (1UL << 24));
    }
}

#if 1
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
#endif

static q24_8_t calculate_voc(uint16_t adc_isense, uint16_t adc_vsense, q24_8_t rwind)
{
    q24_8_t Ih = ((adc_isense * VCC_Q24_8) / ((RSENSE_Q24_8 * ISENSEGAIN_Q24_8) >> 8));
    q24_8_t Vh = ((adc_vsense * VCC_Q24_8) / (RDIV_FACTOR_Q24_8));
    return (Vh + ((Ih * rwind) >> 8));
}

/**
 * adc_results[0,2] are current for motor and shaker, respectively
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

int main()
{
    init_hardware();

    // enable interrupts
    _EINT();

    bopit_gamestate_t gs;
    bopit_init(&gs);

    //uint16_t eighth_note_delay = 32767 - 10;
    TA0CTL |= (1 << 2);
    int16_t tnow = ((int16_t)TA0R) / 128;
    while(1) {
        uint16_t tprev = tnow;
        tnow = ((uint16_t)TA0R) / 128;

        // quick hack to deal with overflow on timers
        uint16_t diff = tnow - tprev;
        if (tnow < tprev) diff += 0x200;

        _DINT();
        uint16_t adcnow[4] = {adc_results[0], adc_results[1], adc_results[2], adc_results[3]};
        _EINT();
        uint16_t voc_motor = (uint16_t)calculate_voc(adcnow[0], adcnow[1], MOTOR_RWIND_Q24_8);
        uint16_t voc_shaker = (uint16_t)calculate_voc(adcnow[2], adcnow[3], SHAKER_RWIND_Q24_8);

        bopit_update_state(&gs, (bopit_user_input_t[]){{voc_motor, voc_shaker, 0}}, diff);

#if 0
        static char dbgstr[] = "ea: ,b: ,t:    ,c: \r\n";
        dbgstr[3] = gs.expected_action + '0';
        dbgstr[7] = gs.beat_state + '0';
        dbgstr[18] = (get_pending_audio_clip(&gs)) ? '1' : '0';
        u16_to_hex(gs.t_now, &dbgstr[11]);
        puts_blocking(dbgstr);
#endif

        audio_clip_t* clip = get_pending_audio_clip(&gs);
        if (clip != NULL) {
            start_audio_dma(clip->audio, clip->audio_len);
            static char dbgstr[] = "b0,L= \r\n";
            dbgstr[1] = digit_to_hexchar(gs.beat_state);
            dbgstr[5] = gs.lost + '0';
            puts_blocking(dbgstr);
        }
    }

#if 1
    volatile uint16_t histogram[1024] = { 0 };
    ADC12CTL0 |= (1 << 1);
    while (1) {
        // wait for interrupt to deliver ADC conversion results
        while(adc_ready == 0);
        adc_ready = 0;


        P8OUT &= ~(1 << 1);
        uint16_t result = adc_results[0];
        histogram[result]++;

        uint16_t voc_motor = (uint16_t)calculate_voc(adc_results[0], adc_results[1], MOTOR_RWIND_Q24_8);
        uint16_t voc_shaker = (uint16_t)calculate_voc(adc_results[2], adc_results[3], SHAKER_RWIND_Q24_8);

        if (voc_motor > (2 * 256)) {
            P1OUT |= (1 << 1);
        } else {
            P1OUT &= ~(1 << 1);
        }

        if (voc_shaker > (2 * 256)) {
            P1OUT |= (1 << 0);
        } else {
            P1OUT &= ~(1 << 0);
        }
    }

    while(1);
#endif
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
    // PWM for audio generation
    // Timer config
    TB0CTL = ((0b00u    << 13) |      // Each TBxCLn latch loads independently.
              (0b11u    << 11) |      // Counter length is 8 bits
              (0b10u    <<  8) |      // Use SMCLK for timer
              (0b00u    <<  6) |      // Input divider is /1
              (0b10u    <<  4) |      // Timer counts in continuous mode; rolls over from 0xff to 0x00.
              (0b0u     <<  2) |      // Don't reset timer
              (0b0u     <<  1) |      // interrupt enable
              (0b0u     <<  0));      // interrupt pending flag

    // Channel 0 is responsible for triggering DMA and resetting channel outputs
    TB0CCTL0 = ((0b00u  << 14) |      // Capture mode is "no capture"
                (0b10u  << 12) |      // Cap/Compare input select is din't care.
                (0b0u   << 11) |      // Synchronize capture source is don't care.
                (0b00u  <<  9) |      // TB0CL0 loads immediately when CCR0 is written.
                (0b0u   <<  8) |      // Compare mode
                (0b000u <<  5) |      // Output mode is don't care.
                (0b0u   <<  4) |      // Capture/Compare interrupt enable
                (0b0000u << 0));      // Bottom 4 bits are all don't care.
    TB0CCR0  = 0x0000u;

    // Channel 3 is output.
    TB0CCTL3 = ((0b00u  << 14) |      // Capture mode is "no capture"
                (0b10u  << 12) |      // Cap/Compare input select is don't care.
                (0b0u   << 11) |      // Synchronize capture source is don't care.
                (0b01   <<  9) |      // TB0CL3 loads when the timer hits 0.
                (0b0    <<  8) |      // Compare mode
                (0b111u <<  5) |      // Output mode: Chan3 output is set when timer hits CCR0 &
                                      // cleared when timer hits CCR3.
                (0b0u   <<  4) |      // Capture/Compare interrupt disable
                (0b0000u << 0));      // Bottom 4 bits are don't care.
    TB0CCR3  = 0x0000u;               // Ultimately, this will be filled by DMA, set to a debug value for now.

    // Use Pin P3.4 for timer B channel TB0.3; see table 6-28 of the datasheet.
    P3SEL0 |=  (1 << 4);
    P3SEL1 &= ~(1 << 4);
    P3DIR  |=  (1 << 4);

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
    // Set up DMA channel 1 to copy from FRAM to timer
    // DMA channel 1 loads data from SPI RXBUF (UCxRXBUF) into TB0CCR3.
    // It should be triggered by TB0CCR0 CCIFG and have lower precedence than DMA channel 0.
    // Alternatively, it could be triggered by SPI TX complete.
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

    // some GPIO pins for debugging: p8.1
    P8SEL0 &= ~(1 << 1);
    P8SEL1 &= ~(1 << 1);
    P8DIR |= (1 << 1);
    P8OUT &= ~(1 << 1);
}

static void start_audio_dma(const uint8_t* src, int32_t len)
{
    // Potentially stop transfer
    DMA1CTL &= ~(0b1u << 4);

    // set up source and length
    __data16_write_addr((uintptr_t)(&DMA1SA), (uintptr_t)src);
    DMA1DA   = (intptr_t)(&TB0CCR3);
    DMA1SZ   = (len > 65535) ? (65535) : len;

    // initiate transfer
    DMA1CTL |= (0b1u << 4);
}
