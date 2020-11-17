#include "game.h"

static const uint32_t SHAKER_BACKLASH = 600;


static void advance_lfsr(uint32_t* num)
{
    int bit = *num & 1;
    *num >>= 1;
    if(bit) {
        *num ^= ((1UL << 31) | (1UL << 29) | (1UL << 25) | (1UL << 24));
    }
}

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

void bopit_update_state(bopit_gamestate_t* gs, const bopit_user_input_t* input, uint32_t dt_ms)
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
            gs->pending_audio = action_to_cliptable_map[gs->expected_action][clip_selection];
        } else if (gs->beat_state == BEAT_ENUM_BEAT_1_5) {
            for (uint8_t i = 0; i < 4; i++, advance_lfsr(&gs->lfsr));
            int clip_selection = (gs->lfsr & 0x03);
            gs->pending_audio = it_clips[clip_selection];
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
    if (button_now && !button_prev) {
        gs->tlastedge_button = gs->tnow;
        button_trig = true;
    }
    if (motor_now && !motor_prev) {
        gs->tlastedge_motor = gs->tnow;
        motor_trig = true;
    }
    if ((shaker_now && !shaker_prev) && ((gs->tnow - gs->tlastedge_shaker) > SHAKER_BACKLASH)) {
        gs->tlastedge_shaker = gs->tnow;
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
    gs->motor_prev = input->motor_voc_q8_8;
    gs->shaker_prev = input->shaker_voc_q8_8;
}

const audio_clip_t* get_pending_audio_clip(gamestate_t* gs)
{
    return gs->pending_audio;
}
