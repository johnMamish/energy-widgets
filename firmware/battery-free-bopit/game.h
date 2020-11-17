#ifndef _GAME_H
#define _GAME_H

#include <stdint.h>

#include "audio_samples.h"

typedef enum {
    BOPIT_ACTION_TWIST = 0,
    BOPIT_ACTION_SHAKE = 1,
    BOPIT_ACTION_BOP = 2,
    BOPIT_ACTION_NUM_ACTIONS = 3
} bopit_action_e;

typedef enum {
    BEAT_ENUM_BEAT_1 = 0,     // "command"
    BEAT_ENUM_BEAT_1_5,       // "it"
    BEAT_ENUM_BEAT_2,         // hi-hat quarter note
    BEAT_ENUM_BEAT_3,         // bass drum eighth note
    BEAT_ENUM_BEAT_3_5,       // bass drum eighth note
    BEAT_ENUM_BEAT_4,          // hi-hat quarter note
    BEAT_ENUM_NUM_BEATS
} beat_enum_e;

typedef struct audio_clip {
    const uint8_t* audio;
    size_t audio_len;
} audio_clip_t;

const audio_clip_t bop_clips[] =
{
    {__assets_bop_1_wav, __assets_bop_1_wav_size},
    {__assets_bop_2_wav, __assets_bop_2_wav_size},
    {__assets_bop_3_wav, __assets_bop_3_wav_size},
    {__assets_bop_4_wav, __assets_bop_4_wav_size}
};

const audio_clip_t shake_clips[] =
{
    {__assets_shake_1_wav, __assets_shake_1_wav_size},
    {__assets_shake_2_wav, __assets_shake_2_wav_size},
    {__assets_shake_3_wav, __assets_shake_3_wav_size},
    {__assets_shake_4_wav, __assets_shake_4_wav_size}
};

const audio_clip_t twist_clips[] =
{
    {__assets_twist_1_wav, __assets_twist_1_wav_size},
    {__assets_twist_2_wav, __assets_twist_2_wav_size},
    {__assets_twist_3_wav, __assets_twist_3_wav_size},
    {__assets_twist_4_wav, __assets_twist_4_wav_size}
};

/**
 * maps values in the bopit_action_e enum to tables containing appropriate audio clips
 */
const audio_clip_t* action_to_cliptable_map[] = { twist_clips, shake_clips, bop_clips };

const audio_clip_t it_clips[] =
{
    {__assets_it_1_wav, __assets_it_1_wav_size},
    {__assets_it_2_wav, __assets_it_2_wav_size},
    {__assets_it_3_wav, __assets_it_3_wav_size},
    {__assets_it_4_wav, __assets_it_4_wav_size}
};

const audio_clip_t dry_kick_clip = { __assets_dry_kick_wav, __assets_dry_kick_wav_size };
const audio_clip_t closed_hi_hat_clip = { __assets_Closed_Hi_Hat_5_wav, __assets_Closed_Hi_Hat_5_wav_size };

bopit_user_input {
    uint16_t motor_voc_q8_8;
    uint16_t shaker_voc_q8_8;
    unsigned int button;
} bopit_user_input_t;

/**
 *
 */
const uint16_t note_length_multipliers[] = {1, 1, 2, 1, 1, 2, 0xffff};

/**
 * Schedule of measure numbers and quarter note lengths in millis.
 */
const uint16_t speed_schedule[][2] =
{
    {0,  250},      // 120 BPM
    {12, 227},      // 132 BPM
    {24, 208},      // 144 BPM
    {36, 187},      // 160 BPM
    {48, 174},      // 172 BPM
    {60, 163},      // 184 BPM
    {72, 156},      // 192 BPM
    {84, 150},      // 200 BPM
    {0xffff, 100}
};

/**
 * schedule of all sounds except the command and the word "it"
 */
const audio_clip_t* sound_schedule[] =
{
    NULL, NULL,                         // beat 1
    &closed_hi_hat_clip,                //      2
    &dry_kick_clip, &dry_kick_clip,     //      3
    &closed_hi_hat_clip                 //      4
};

uint16_t motor_hystersis[] = { 0x300, 0x200 };
uint16_t shaker_hystersis[] = { 0x500, 0x200 };


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
void bopit_update_state(bopit_gamestate_t* gs, const bopit_user_input_t* input, int16_t dt_ms);

/**
 *
 */
const audio_clip_t* get_pending_audio_clip(gamestate_t* gs);

/**
 *
 */


#endif
