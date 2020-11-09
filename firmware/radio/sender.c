//sender
//msp430fr5994
#include <msp430.h>
#include "msprf24.h"
#include "nrf_userconfig.h"

#if 0 //powered by buttons

void red(void) {P1OUT |= BIT0;}
void green(void) {P1OUT |= BIT1;}
void both(void) {P1OUT |= BIT0 | BIT1;}

/*
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
*/



int main(void)
{
    WDTCTL = WDTHOLD | WDTPW;
    PM5CTL0 &= ~LOCKLPM5; //can this be up here?

    __bis_SR_register(GIE);

    // todo: increase clock frequency?
    // notes: see CSCTL1 register and Figure 3-1: clock system bus diagram; MCLK and SMCLK both use
    // DCO  / 8 at startup; DCO is set to 8MHz at startup.

    CSCTL0_H = CSKEY_H; //unlock
    CSCTL1 = DCOFSEL_0; //1MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1; //keep everything at 1MHz
    CSCTL0_H = 0; //lock


    P1REN &= ~(BIT1 | BIT0); //leds for debugging
    P1DIR |= BIT1 | BIT0;
    P1OUT &= ~(BIT1 | BIT0);

    /*
    int i;
    // make button read pins inputs
    for (i = 0; i < (2 * NUM_BUTTONS); i++)
        *(button_read_ports[i] + button_read_mmio_offsets[i] + OFS_P1DIR) &= ~(1 << button_read_pins[i]);

    // make button write pins outputs, and initialize the output to 0
    for (i = 0; i < (2 * NUM_BUTTONS); i++) {
        *(button_clear_ports[i] + button_clear_mmio_offsets[i] + OFS_P1OUT) &= ~(1 << button_clear_pins[i]);
        *(button_clear_ports[i] + button_clear_mmio_offsets[i] + OFS_P1DIR) |= (1 << button_clear_pins[i]);
    }

    int char_map[] = { '1', -1, '2', -1, '3', -1, '4', -1 };
    int char_to_send = -2;

    PM5CTL0 &= ~LOCKLPM5; //why is this here?

    // figure out which button was pressed
    uint8_t button_states[] = { 0, 0, 0, 0 };
    for (i = 0; i < (2 * NUM_BUTTONS); i++) {
        if (!(*(button_read_ports[i] + button_read_mmio_offsets[i] + OFS_P1IN) & (1 << button_read_pins[i]))) {
            button_states[i] = 1;
            char_to_send = char_map[i];
        }
        *(button_clear_ports[i] + button_clear_mmio_offsets[i] + OFS_P1OUT) |= (1 << button_clear_pins[i]);
    }

    if (button_states[0]+button_states[1]+button_states[2]+button_states[3] != 1) {
        both(); //error
    } else if (button_states[0]) {
        green();
    } else if (button_states[1]) {
        red();
    } else if (button_states[2]) {

    } else if (button_states[3]) {

    }

    */

    // send - straight shot, no attention paid to rf_irq
    rf_crc         = 0;
    rf_addr_width  = 3;
    rf_speed_power = RF24_SPEED_MIN | RF24_POWER_MAX;
    rf_channel     = 120;
    uint8_t addr[3] = { 0x11, 0x22, 0x33 };
    uint8_t payload[] = { 0x55 };
    msprf24_init();
    w_tx_addr(addr);
    w_tx_payload_noack(sizeof(payload), payload);
    msprf24_activate_tx();
    LPM4;
    while(1){}
}



#else //plugged in

void red(void)
{
    P1OUT |= BIT0;
    __delay_cycles(200000);
    P1OUT &= ~BIT0;
}

void green(void)
{
    P1OUT |= BIT1;
    __delay_cycles(200000);
    P1OUT &= ~BIT1;
}

void both(void)
{
    P1OUT |= BIT0 | BIT1;
    __delay_cycles(900000);
    P1OUT &= ~(BIT0 | BIT1);
    __delay_cycles(100000);
}

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

    P1REN &= ~(BIT1 | BIT0); //leds for debugging
    P1DIR |= BIT1 | BIT0;
    P1OUT &= ~(BIT1 | BIT0);

    // weird - this stops working when crc is enabled on both sides
    //         I believe this indicates the presence of a ton of noise?
    rf_crc         = 0;//RF24_EN_CRC | RF24_CRCO; // CRC enabled, 16-bit
    rf_addr_width  = 3;
    rf_speed_power = RF24_SPEED_MIN | RF24_POWER_MAX;
    rf_channel     = 120;

    uint8_t payload[] = {0x55};
    uint8_t addr[3] = {0x11, 0x22, 0x33};


#if 1 //noack
    msprf24_init();
    w_tx_addr(addr);

    while (1) {
        __delay_cycles(200000);
        w_tx_payload_noack(sizeof(payload), payload);
        msprf24_activate_tx();
        LPM4; //resume via P8_IRQ
        if (rf_irq & RF24_IRQ_FLAGGED) {
            rf_irq &= ~RF24_IRQ_FLAGGED;
            msprf24_get_irq_reason();
            if (rf_irq & RF24_IRQ_TX)
                green();
            //TXFAILED can't be set for noack
        }
        msprf24_irq_clear(RF24_IRQ_MASK); //clear all
    }

#else //ack
    msprf24_init();
    msprf24_set_pipe_packetsize(0,sizeof(payload));
    msprf24_open_pipe(0, 1); //pipe0, autoack on
    msprf24_standby(); //is there a reason to have this here? only purpose is power saving, right? not applicable bc want to send immediately when want to save power
    w_tx_addr(addr);
    w_rx_addr(0, addr); //receive autoacks to pipe0, "you must write the TX address into the RX address for pipe#0"

    while (1) {
        __delay_cycles(200000);
        w_tx_payload(sizeof(payload), payload);
        msprf24_activate_tx();
        LPM4; //resume via P8_IRQ
        if (rf_irq & RF24_IRQ_FLAGGED) {
            rf_irq &= ~RF24_IRQ_FLAGGED;
            msprf24_get_irq_reason();
            if (rf_irq & RF24_IRQ_TX)
                green();
            if (rf_irq & RF24_IRQ_TXFAILED)
                red();
        }
        msprf24_irq_clear(RF24_IRQ_MASK); //clear all
    }

#endif
} //end main



#endif
