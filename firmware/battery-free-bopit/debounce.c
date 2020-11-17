#include "debounce.h"

bool debounce_button(debounce_state_t* db, bool button, uint32_t time_now)
{
    if ((button == true) && (db->prev_sample == false)) {
        db->last_edge_time = time_now;
    }

    if ((button == false) && (db->prev_sample == true)) {
        db->last_edge_time = time_now;
    }

    db->prev_sample = button;

    if ((time_now - db->last_edge_time) > db->debounce_interval) {
        db->button_out = button;
    }

    return db->button_out;
}
