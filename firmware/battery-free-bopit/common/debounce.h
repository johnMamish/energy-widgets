#ifndef _DEBOUNCE_H
#define _DEBOUNCE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct debounce_state
{
    bool prev_sample;
    uint32_t last_edge_time;
    uint32_t debounce_interval;
    bool button_out;
} debounce_state_t;

/**
 * This function updates the given debounce_state struct and a debounced button true/false
 */
bool debounce_button(debounce_state_t* db, bool button, uint32_t time_now);

#endif
