#ifndef _GAME_H
#define _GAME_H

#include <stdint.h>

#include "audio_samples.h"
#include "debounce.h"

typedef enum {
    BOPIT_ACTION_TWIST = 0,
    BOPIT_ACTION_SHAKE = 1,
    BOPIT_ACTION_BOP = 2,
    BOPIT_ACTION_NUM_ACTIONS = 3
} bopit_action_e;

typedef enum {
    BEAT_ENUM_BEAT_1 = 0,     // "command"
    BEAT_ENUM_BEAT_1_5 = 1,       // "it"
    BEAT_ENUM_BEAT_2 = 2,         // hi-hat quarter note
    BEAT_ENUM_BEAT_3 = 3,         // bass drum eighth note
    BEAT_ENUM_BEAT_3_5 = 4,       // bass drum eighth note
    BEAT_ENUM_BEAT_4 = 5,          // hi-hat quarter note
    BEAT_ENUM_NUM_BEATS
} beat_enum_e;

typedef struct audio_clip {
    const uint8_t* audio;
    size_t audio_len;
} audio_clip_t;

typedef struct bopit_user_input {
    uint16_t motor_voc_q8_8;
    uint16_t shaker_voc_q8_8;
    unsigned int button;
} bopit_user_input_t;

typedef struct bopit_gamestate {
    int32_t t_now;
    int32_t t_next_beat;

    /**
     * 0 if the game is still active, 1 if the player has lost
     */
    int lost;

    /**
     * Holds the next expected action. Generated on beat 2 of each measure after a successful user
     * action.
     */
    uint16_t ms_per_eighth_note;
    uint16_t measure_number;
    bopit_action_e expected_action;
    int32_t t_this_action;
    int32_t t_measure_start;
    int32_t action_window;                 // tolerance in time-domain for beat detection
    beat_enum_e beat_state;

    /**
     * linear-feedback shift register for random number generation
     */
    uint32_t lfsr;

    /**
     * Variables for keeping track of previous UI state
     */
    debounce_state_t bop_debouncer;
    bool button_prev, motor_prev, shaker_prev;
    int32_t tlastedge_button;
    int32_t tlastedge_motor;
    int32_t tlastedge_shaker;

    /**
     * If this is null, there's no sound ready to play.
     */
    const audio_clip_t* pending_audio;
} bopit_gamestate_t;

void bopit_init(bopit_gamestate_t* gs);

/**
 *
 */
void bopit_update_state(bopit_gamestate_t* gs, const bopit_user_input_t* input, uint16_t dt_ms);

/**
 *
 */
const audio_clip_t* get_pending_audio_clip(bopit_gamestate_t* gs);

/**
 *
 */


#endif
