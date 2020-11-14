#include "game.h"

void bopit_init(bopit_gamestate_t* gs)
{
    gs->t_now = 0;
    gs->t_next_beat = 0;
    gs->lost = 0;

    gs->ms_per_eighth_note = speed_schedule[0][1];
    gs->measure_number = 0;
    gs->expected_action = BOPIT_ACTION_TWIST;
    gs->action_window = 100;                    // 100 milliseconds of slop for UI is ~16th note @ 120bpm
    gs->t_this_action = 0xffffffff - gs->action_window;
    gs->beat_state = BEAT_ENUM_BEAT_2;

    gs->lfsr = 0xa5ce5b3a;  // TODO: seed properly
    gs->motor_state_prev = 0;
    gs->shaker_state_prev = 0;
    gs->button_state_prev = 0;
}

void bopit_update_state(bopit_gamestate_t* gs, const bopit_user_input_t input, uint16_tuint32_t dt_ms)
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


    // check to see if the person missed a command
    if (gs->t_now >= (gs->t_this_action + (gs->action_window / 2))) {
        // check beat times

        // if (missed beat) {
        //     gs->lost = xx;
        //     disable interrupts
        //     play oopsie noise
        //     while(1);
        // }
    }
}

const audio_clip_t* get_pending_audio_clip(gamestate_t* gs)
{
    return gs->pending_audio;
}
