#include "game.h"

static const uint32_t SHAKER_BACKLASH = 600;

static audio_clip_t bop_clips[4];
static audio_clip_t shake_clips[4];
static audio_clip_t twist_clips[4];
static audio_clip_t it_clips[4];

static audio_clip_t dry_kick_clip;
static audio_clip_t closed_hi_hat_clip;


/**
 * maps values in the bopit_action_e enum to tables containing appropriate audio clips
 */
const audio_clip_t* action_to_cliptable_map[] = { twist_clips, shake_clips, bop_clips };

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

static void advance_lfsr(uint32_t* num)
{
    int bit = *num & 1;
    *num >>= 1;
    if(bit) {
        *num ^= ((1UL << 31) | (1UL << 29) | (1UL << 25) | (1UL << 24));
    }
}

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

uint16_t motor_hystersis[] = { 0x300, 0x200 };
uint16_t shaker_hystersis[] = { 0x500, 0x200 };

void bopit_init(bopit_gamestate_t* gs)
{
    gs->t_now = 0;
    gs->t_next_beat = 0;
    gs->lost = 0;

    gs->ms_per_eighth_note = speed_schedule[0][1];
    gs->measure_number = 0;
    gs->expected_action = BOPIT_ACTION_TWIST;
    gs->action_window = 100;                    // 100 milliseconds of slop for UI is ~16th note @ 120bpm
    gs->t_this_action = 0x7ffffff - gs->action_window; // unreachably far in the future to initialize
    gs->beat_state = BEAT_ENUM_BEAT_2;

    gs->lfsr = 0xa5ce5b3a;  // TODO: seed properly
    for (int i = 0; i < sizeof(gs->bop_debouncer); ((uint8_t*)(&gs->bop_debouncer))[i++] = 0);
    gs->bop_debouncer.debounce_interval = 10;
    gs->motor_prev = 0;
    gs->shaker_prev = 0;

    // These can't be initialized as compile-time constants because the ..._size variables are all determined by
    // sizeof()'s of arrays in an external compilation unit. There are ways to resolve this both
    // on the c source-generation side (./tools/audio_to_header.py) and on the linker side, but this
    // seemed like the eaiest
    bop_clips[0].audio = __assets_bop_1_wav; bop_clips[0].audio_len = __assets_bop_1_wav_size;
    bop_clips[1].audio = __assets_bop_2_wav; bop_clips[1].audio_len = __assets_bop_2_wav_size;
    bop_clips[2].audio = __assets_bop_3_wav; bop_clips[2].audio_len = __assets_bop_3_wav_size;
    bop_clips[3].audio = __assets_bop_4_wav; bop_clips[3].audio_len = __assets_bop_4_wav_size;
    shake_clips[0].audio = __assets_shake_1_wav; shake_clips[0].audio_len = __assets_shake_1_wav_size;
    shake_clips[1].audio = __assets_shake_2_wav; shake_clips[1].audio_len = __assets_shake_2_wav_size;
    shake_clips[2].audio = __assets_shake_3_wav; shake_clips[2].audio_len = __assets_shake_3_wav_size;
    shake_clips[3].audio = __assets_shake_4_wav; shake_clips[3].audio_len = __assets_shake_4_wav_size;
    twist_clips[0].audio = __assets_twist_1_wav; twist_clips[0].audio_len = __assets_twist_1_wav_size;
    twist_clips[1].audio = __assets_twist_2_wav; twist_clips[1].audio_len = __assets_twist_2_wav_size;
    twist_clips[2].audio = __assets_twist_3_wav; twist_clips[2].audio_len = __assets_twist_3_wav_size;
    twist_clips[3].audio = __assets_twist_4_wav; twist_clips[3].audio_len = __assets_twist_4_wav_size;
    it_clips[0].audio = __assets_it_1_wav; it_clips[0].audio_len = __assets_it_1_wav_size;
    it_clips[1].audio = __assets_it_2_wav; it_clips[1].audio_len = __assets_it_2_wav_size;
    it_clips[2].audio = __assets_it_3_wav; it_clips[2].audio_len = __assets_it_3_wav_size;
    it_clips[3].audio = __assets_it_4_wav; it_clips[3].audio_len = __assets_it_4_wav_size;
    dry_kick_clip.audio = __assets_dry_kick_wav; dry_kick_clip.audio_len = __assets_dry_kick_wav_size;
    closed_hi_hat_clip.audio = __assets_dry_kick_wav; closed_hi_hat_clip.audio_len = __assets_dry_kick_wav_size;
}

/**
 * utility function
 */
static bopit_action_e edges_to_bopit_action_e(bool button_edge, bool motor_edge, bool shaker_edge)
{
    if (button_edge && !motor_edge && !shaker_edge) return BOPIT_ACTION_BOP;
    if (!button_edge && motor_edge && !shaker_edge) return BOPIT_ACTION_TWIST;
    if (!button_edge && !motor_edge && shaker_edge) return BOPIT_ACTION_SHAKE;
    return BOPIT_ACTION_NUM_ACTIONS;
}

void bopit_update_state(bopit_gamestate_t* gs, const bopit_user_input_t* input, uint16_t dt_ms)
{
    gs->t_now += dt_ms;

    // update state variables as appropriate on beats
    if (gs->t_now >= gs->t_next_beat) {
        // advance beat state variables
        gs->beat_state++;
        if (gs->beat_state == BEAT_ENUM_NUM_BEATS) {
            gs->beat_state = BEAT_ENUM_BEAT_1;
        }

        // speed up if appropriate
        if (gs->beat_state == BEAT_ENUM_BEAT_1) {
            gs->measure_number++;
            int sched_slot;
            for (sched_slot = 0; speed_schedule[sched_slot][0] > gs->measure_number; sched_slot++);
            gs->ms_per_eighth_note = speed_schedule[sched_slot][1];
        }

        // select proper sound to make pending
        if (gs->beat_state == BEAT_ENUM_BEAT_1) {
            for (uint8_t i = 0; i < 4; i++, advance_lfsr(&gs->lfsr));
            int clip_selection = (gs->lfsr & 0x03);
            gs->pending_audio = &action_to_cliptable_map[gs->expected_action][clip_selection];
        } else if (gs->beat_state == BEAT_ENUM_BEAT_1_5) {
            for (uint8_t i = 0; i < 4; i++, advance_lfsr(&gs->lfsr));
            int clip_selection = (gs->lfsr & 0x03);
            gs->pending_audio = &it_clips[clip_selection];
        } else {
            gs->pending_audio = sound_schedule[gs->beat_state];
        }

        // calculate next beat time
        gs->t_next_beat += note_length_multipliers[gs->beat_state] * gs->ms_per_eighth_note;

        // If we just played beat 4, schedule the next expected UI action
        if (gs->beat_state == BEAT_ENUM_BEAT_4) {
            for (uint8_t i = 0; i < 4; i++, advance_lfsr(&gs->lfsr));

            gs->expected_action = ((uint8_t)gs->lfsr) % BOPIT_ACTION_NUM_ACTIONS;
            gs->t_this_action = gs->t_next_beat;
        }

    } else {
        // clear pending sound
        gs->pending_audio = NULL;
    }

    // update ui beattimes
    bool button_now = debounce_button(&gs->bop_debouncer, (input->button == 0), gs->t_now);
    bool motor_now = (input->motor_voc_q8_8 > motor_hystersis[(gs->motor_prev ? 1 : 0)]);
    bool shaker_now = (input->shaker_voc_q8_8 > shaker_hystersis[(gs->shaker_prev ? 1 : 0)]);

    bool button_trig = false;
    bool motor_trig = false;
    bool shaker_trig = false;
    if (button_now && !gs->button_prev) {
        gs->tlastedge_button = gs->t_now;
        button_trig = true;
    }
    if (motor_now && !gs->motor_prev) {
        gs->tlastedge_motor = gs->t_now;
        motor_trig = true;
    }
    if ((shaker_now && !gs->shaker_prev) && ((gs->t_now - gs->tlastedge_shaker) > SHAKER_BACKLASH)) {
        gs->tlastedge_shaker = gs->t_now;
        shaker_trig = true;
    }


    // check to see if the person missed a command
    if (button_trig || motor_trig || shaker_trig ||
        (gs->t_now >= (gs->t_this_action + (gs->action_window / 2)))) {
        // check that the person did the right one
        if ((edges_to_bopit_action_e(button_trig, motor_trig, shaker_trig) != gs->expected_action) ||
            (gs->t_now > (gs->t_this_action + (gs->action_window / 2))) ||
            (gs->t_now < (gs->t_this_action - (gs->action_window / 2)))) {
            gs->lost = 1;
        }
    }

    gs->button_prev = button_now;
    gs->motor_prev = motor_now;
    gs->shaker_prev = shaker_now;
}

const audio_clip_t* get_pending_audio_clip(bopit_gamestate_t* gs)
{
    return gs->pending_audio;
}
