#include "msp430fr5994.h"

#include <stdint.h>

/**
 * Convenience function that all of the hardware init functions are being dumped into
 */
static void init_hardware();

static uint16_t dma_src_word = 0x80;
static uint16_t dma_dest_word = 0x0000;
int main()
{
    init_hardware();

    // start DMA1
    DMA1CTL |= (0b1u << 4);
    //DMA1CTL |= (1 << 0);
    while(1) {
        if (DMA1CTL & (1 << 3)) {
            DMA1CTL &= ~(1 << 3);
            dma_src_word = (dma_src_word == 0x40) ? (0x00) : (dma_src_word + 1);
        }
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

    // Change SMCLK to be 8MHz / 1 instead of 8MHz / 8. Clock control registers need to be unlocked
    // by writing a key value to CSCTL0 first.
    CSCTL0  = (0xa5u << 8);
    CSCTL3 &= ~(0b111u << 4); CSCTL3 |= (0b000u << 4);

    ////////////////////////////////////////////////////////////////
    // PWM for audio generation
    // Timer config
    TB0CTL = ((0b00u    << 13) |      // Each TBxCLn latch loads independently.
              (0b11u    << 11) |      // Counter length is 8 bits
              (0b10u    <<  8) |      // Use SMCLK for timer
              (0b00u    <<  6) |      // Input divider is /1
              (0b10u    <<  4) |      // Timer counts in continuous mode; rolls over from 0xff to 0x00.
              (0b0u     <<  2) |      // Don't reset timer
              (0b1u     <<  1) |      // interrupt enable
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
    TB0CCR3  = 0x0037u;               // Ultimately, this will be filled by DMA, set to a debug value for now.

    // Use Pin P3.4 for timer B channel TB0.3; see table 6-28 of the datasheet.
    P3SEL0 |=  (1 << 4);
    P3SEL1 &= ~(1 << 4);
    P3DIR  |=  (1 << 4);

    ////////////////////////////////////////////////////////////////
    // TODO: Set up eUSCI


    ////////////////////////////////////////////////////////////////
    // TODO: Set up DMA channel 1.
    // DMA channel 1 loads data from SPI RXBUF (UCxRXBUF) into TB0CCR3.
    // It should be triggered by TB0CCR0 CCIFG and have lower precedence than DMA channel 0.
    // Alternatively, it could be triggered by SPI TX complete.
    DMA1SA   = (intptr_t)(&dma_src_word);
    DMA1DA   = (intptr_t)(&TB0CCR3);
    //DMA1DA   = (intptr_t)(&dma_dest_word);
    DMA1SZ   = 1;
    DMACTL0 &= ~(0b11111u << 8); DMACTL0 |= DMA1TSEL__TB0CCR0;
    DMA1CTL  = ((0b100u  << 12) |     // DMA xfer mode: repeated single xfer
                (0b00u   << 10) |     // Don't increment destination.
                (0b00u   <<  8) |     // Don't increment source.
                (0b0u    <<  7) |     // Destination is a word, not a byte.
                (0b0u    <<  6) |     // Source is a word, not a byte.
                (0b1u    <<  5) |     // level sensitive DMA trigger
                (0b0u    <<  4) |     // DMA Enable
                (0b0100u <<  0));     // Bottom 4 bits are flags / don't care.


    ////////////////////////////////////////////////////////////////
    // TODO: Set up DMA channel 'b'.
    // DMA channel 'b' loads a don't-care constant from memory into the SPI TXBUF to initiate a
    // transfer which will load data into UCxRXBUF.
    // It should be triggered by TB0CCR0 CCIFG and have higher precedence than DMA channel 'a'.


}