#include "samd21g18a.h"

#include "audio_samples.h"
#include "game.h"

void init_hardware();
void start_audio_dma(const uint8_t* datas, int len);


static DmacDescriptor base_descriptor __attribute__((aligned(16)));

// The DMAC uses this as scratchpad space to store a DMAC descriptor which changes as the
// DMAC transfers corresponding to the descriptor are filled out.
static DmacDescriptor dmac_writeback   __attribute__((aligned(16)));

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

/**
 *
 */
int main() {
    init_hardware();

    //volatile int i = foo[0x2ff];
    //start_audio_dma(__assets_dry_kick_wav, __assets_dry_kick_wav_size);

    bopit_gamestate_t gs;
    bopit_init(&gs);
    uint16_t tnow = TC3->COUNT16.COUNT.reg;
    while(1) {
        uint16_t tprev = tnow;
        tnow = TC3->COUNT16.COUNT.reg;

        uint16_t diff = tnow - tprev;

        bopit_update_state(&gs, (bopit_user_input_t[]){{0, 0, 0}}, diff);

        audio_clip_t* clip = get_pending_audio_clip(&gs);
        if (clip != NULL) {
            start_audio_dma(clip->audio, clip->audio_len);
        }
    }
    return (0);
}

void init_hardware()
{
    // Configure cache and wait states
    NVMCTRL->CTRLB.reg = ((0b01 << 16) |    // low-power read mode
                              (0b0000 << 1));   // 0 NVM wait states (table 36-39)

    // change OSC8M output divider from 8 to 1.
    SYSCTRL->OSC8M.reg &= ~(0b11ul << 8);

    // Adafruit metro uart tx pin is PA10 / SERCOM0/PAD[2]
    // select function C for pin PA10
    PORT->Group[0].PMUX[(10 >> 1)].reg &= ~(0x0f << 0);
    PORT->Group[0].PMUX[(10 >> 1)].reg |=  (0x02 << 0);
    PORT->Group[0].PINCFG[10].reg = 0x01;

    ////////////////////////////////////////////////////////////////////////////
    // Initialize sercom0 as a UART running at a 115200 baud off of OSC8M
    // enable sercom0's clock.
    PM->APBCMASK.reg |= (1 << 2);

    // route GCLK0 (which contains OSC8M) undivided to sercom0 and sercomx_slow
    GCLK->CLKCTRL.reg = (1 << 14) | (0 << 8) | (0x13 << 0);
    GCLK->CLKCTRL.reg = (1 << 14) | (0 << 8) | (0x14 << 0);
    SERCOM0->USART.CTRLA.reg = ((1 << 30) |    // DORD: LSB first
                                (3 << 20) |    // RXPO: SERCOM[3] is RX
                                (1 << 16) |    // TXPO: SERCOM[2] is TX
                                (1 << 13) |    // SAMPR: 16x oversamp w/ fractional
                                // baud rate gen
                                (1 << 2));     // use internal clock

    // Fractional division of 8MHz input clock by (16 * (4 + (3 / 8)).
    SERCOM0->USART.BAUD.reg = (3 << 13) | (4 << 0);

    // enable RX and TX
    SERCOM0->USART.CTRLB.reg = ((1 << 17) | (1 << 16));

    // enable SERCOM0
    SERCOM0->USART.CTRLA.reg |= (1 << 1);

    ////////////////////////////////////////////////////////////////////////////
    // Select function F (TCC0/WO[4]) for PA14
    PORT->Group[0].WRCONFIG.reg = ((0 << 31) |   // configure a pin in range 0 - 15
                                   (1 << 30) |   // change pincfg for corresponding pin
                                   (1 << 28) |   // change PMUX for corresponding pin
                                   (5 << 24) |   // choose function F for pmux
                                   (0 << 22) |   // normal drive strength
                                   (0 << 18) |   // disable pull resistor
                                   (0 << 17) |   // disable input
                                   (1 << 16) |   // enable PMUX selection
                                   (1 << 14));   // apply this config to pin 14

    ////////////////////////////////////////////////////////////////////////////
    // Setup TC3 to be driven by clkgen 1 and tick 1 time per millisecond
    PM->APBCMASK.bit.TC3_ = 1;

    GCLK->GENCTRL.reg = ((1 << 21) |    // clkgen 1 should run in standby
                         (0 << 20) |    // clkgen 1 is divided by GENDIV.DIV
                         (1 << 17) |    // improve duty cycle for odd division factors
                         (1 << 16) |    // enable the clock generator
                         (6 << 8)  |    // source is OSC8M
                         (1 << 0));     // clkgen 1 is our config target
    GCLK->GENDIV.reg  = ((125 << 8) | (1 << 0));             // set up x125 divider
    GCLK->CLKCTRL.reg = (1 << 14) | (1 << 8) | (0x1b << 0);

    TC3->COUNT16.CTRLA.bit.RUNSTDBY = 1;
    TC3->COUNT16.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV64_Val;
    TC3->COUNT16.CTRLA.bit.MODE = TC_CTRLA_MODE_COUNT16_Val;

    TC3->COUNT16.READREQ.bit.ADDR = 0x10;
    TC3->COUNT16.READREQ.bit.RCONT = 1;

    TC3->COUNT16.CTRLA.bit.ENABLE = 1;

    ////////////////////////////////////////////////////////////////////////////
    // Configure TCC0 to output audio PWM signal on WO[4]
    PM->APBCMASK.reg |= (1 << 8);

    // route GCLK0 (which carries OSC8M) undivided to TCC0
    GCLK->CLKCTRL.reg = (1 << 14) | (0 << 8) | (0x1a << 0);

    //
    TCC0->CTRLA.reg = ((0 << 12) |   // prescaler sync: counter is reloaded on GCLK
                       (1 << 11) |   // RUNSTDBY: run in standby
                       (0 << 8)  |   // PRESCALER: divide by 1 prescaler
                       (0 << 5));    // RESOLUTION: don't perform dithering

    TCC0->WAVE.reg  = ((0 << 4)  |   // RAMP: RAMP1 mode (typical single-slope PWM)
                       (2 << 0));    // Normal PWM waveform generation

    TCC0->PER.reg   = 0xff;
    TCC0->CCB[0].reg = 0x70;

    // Enable TCC0
    TCC0->CTRLA.reg |= (1 << 1);

    ////////////////////////////////////////////////////////////////////////////
    // Configure DMAC to
    PM->AHBMASK.bit.DMAC_ = 1;

    // Do a software reset of DMAC
    DMAC->CTRL.reg |= (1 << 0);
    while(DMAC->CTRL.reg & (1 << 0));

    // Set descriptor base and writeback memory
    DMAC->BASEADDR.reg = (uint32_t)&base_descriptor;
    DMAC->WRBADDR.reg  = (uint32_t)&dmac_writeback;

    // select highest QoS level
    DMAC->QOSCTRL.reg |= 0x3f;

    // We're gonna use channel 0, so that's the one we want to configure.
    DMAC->CHID.reg = 0;

    // Send one beat on each trigger
    DMAC->CHCTRLB.reg = ((2 << 22) |     // Send one 'beat' on each trigger
                               (0x0d << 8) |   // use TCC0 OVF as trigger
                               (3 << 5) |      // Channel priority level 3 (probably unnecessary)
                               (0 << 4) |      // No channel event output enable
                               (1 << 3));      // enable channel event input

    // Enable transfers for all levels
    DMAC->CTRL.reg |= (0x0f << 8);

    // Enable the DMA
    DMAC->CTRL.reg |= (1 << 1);

}

void start_audio_dma(const uint8_t* data, int len)
{
    // disable DMAC channel so it can be configured
    DMAC->CHID.reg = 0;
    DMAC->CHCTRLA.reg &= ~(1 << 1);
    while(DMAC->CHCTRLA.reg & (1 << 1));

    // setup descriptor
    base_descriptor.BTCTRL.reg = ((0 << 13) |   // stepsize: increment by 1 * beatsze
                                  (1 << 12) |   // stepsize applies to source addr
                                  (0 << 11) |   // destination addr increment disabled
                                  (1 << 10) |   // source address increment enabled
                                  (0 << 8)  |   // beat size is 1 byte
                                  (0 << 3)  |   // no action should be taken after block xfer complete
                                  (0 << 1)  |   // no event generation)
                                  (1 << 0));    // mark descriptor valid

    base_descriptor.BTCNT.reg = len;
    base_descriptor.SRCADDR.reg = ((uint32_t)data) + len;
    base_descriptor.DSTADDR.reg = (uint32_t)&TCC0->CCB[0];
    base_descriptor.DESCADDR.reg = 0;  // setting this to NULL terminates

    DMAC->CHID.reg = 0;
    DMAC->CHCTRLA.reg |= (1 << 1);
    while(!(DMAC->CHCTRLA.reg & (1 << 1)));
}
