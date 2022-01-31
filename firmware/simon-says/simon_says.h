#ifndef _SIMON_SAYS
#define _SIMON_SAYS

#include <stdint.h>

typedef uint8_t simon_says_user_input_t;

typedef enum {
    PLAYER_ACTIVE_GENERATOR_ACTIVE,
    PLAYER_ACTIVE_PLAYER_ACTIVE
} player_active_e;

typedef struct simon_says_gamestate {
    /// A simon says sequence is determined by an LFSR.
    uint32_t lfsr_seed;
    uint32_t lfsr;

    uint32_t t_now;
    int32_t t_last_press;

    /// Which step is the generator / player at?
    uint16_t level_num;
    uint16_t step_num;
    uint8_t  player_active;

    // goes to '1' if the game has been lost
    uint8_t game_lost;

    // output bitmap; up to the user code to interpret this and set LEDs/audio appropriately.
    // |-- If this bit is high, a "user lost" sound/graphic should show
    // |  |--|--|----|--|--|--|-- These bits correspond to buttons
    // V  V  V  V    V  V  V  V
    // 0  0  0  0    0  0  0  0
    uint8_t output;

    // set by user - how many ms should the display stay active after the button was pressed?
    uint32_t output_high_time;
} simon_says_gamestate_t;


void simon_says_init(simon_says_gamestate_t* gs,
                     uint32_t lfsr_seed);

void simon_says_advance_gamestate(simon_says_gamestate_t* gs,
                                  simon_says_user_input_t user_input,
                                  uint16_t dt_ms);

#endif
