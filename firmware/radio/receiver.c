//receiver
//msp430fr5969 (for now)
#include <msp430.h>
#include "msprf24.h"
#include "nrf_userconfig.h"

const uint8_t pipeid = 1; //1 to make clear that this isn't autoack rx

void red(void)
{
    P4OUT |= BIT6;
    __delay_cycles(200000);
    P4OUT &= ~BIT6;
}

void green(void)
{
    P1OUT |= BIT0;
    __delay_cycles(200000);
    P1OUT &= ~BIT0;
}

void both(void){
    P1OUT |= BIT0;
    P4OUT |= BIT6;
    __delay_cycles(900000);
    P1OUT &= ~BIT0;
    P4OUT &= ~BIT6;
    __delay_cycles(100000);}

int main(void)
{
    WDTCTL = WDTHOLD | WDTPW;
    PM5CTL0 &= ~LOCKLPM5;

    __bis_SR_register(GIE);

    CSCTL0_H = CSKEY_H; //unlock
    CSCTL1 = DCOFSEL_0; //1MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1; //keep everything at 1MHz
    CSCTL0_H = 0; //lock

    P1DIR |= BIT0; //leds
    P4DIR |= BIT6;
    P1OUT &= ~BIT0;
    P4OUT &= ~BIT6;

    //P1DIR |= BIT4; //CE - set it and forget it. this causes something to fail?
    //P1OUT |= BIT4;

    rf_crc         = 0; //RF24_EN_CRC;// | RF24_CRCO; // CRC enabled, 16-bit
    rf_addr_width  = 3;
    rf_speed_power = RF24_SPEED_MIN | RF24_POWER_MAX;
    rf_channel     = 120;

    uint8_t payload[] = {0xAA};
    uint8_t addr[3] = {0x11, 0x22, 0x33};

    msprf24_init();
    if (!msprf24_is_alive())
        while(1){both();} //error

    msprf24_set_pipe_packetsize(pipeid, sizeof(payload)); //pipeid, packetsize in bytes where 0 is dynamic (r_rx_peek_payload_size() needed if dynamic)
#if 1 //autoack
    msprf24_open_pipe(pipeid, 0);
#else
    msprf24_open_pipe(pipeid, 1);
#endif
    if (!msprf24_pipe_isopen(pipeid))
        while(1) {both();} //error

    w_rx_addr(pipeid, addr);

    if (!(RF24_QUEUE_RXEMPTY & msprf24_queue_state())) {
        flush_rx();
    }

    msprf24_activate_rx();
    LPM4;

    while(1){
        //if (msprf24_current_state() != RF24_STATE_PRX || !msprf24_is_alive())
        //    while(1){both();} //error

        if (rf_irq & RF24_IRQ_FLAGGED) {
            rf_irq &= ~RF24_IRQ_FLAGGED;
            msprf24_get_irq_reason();
            green();
            if (rf_irq & RF24_IRQ_RX || msprf24_rx_pending()) {
                r_rx_payload(sizeof(payload), payload);
                msprf24_irq_clear(RF24_IRQ_RX);
                red();
                if (payload[0] == 0x55) //confirm correct
                    green();
            }
        }
        payload[0] = 0xAA;
        LPM4;

    }
}

