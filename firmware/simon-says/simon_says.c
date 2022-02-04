#include "simon_says.h"
#include "uart_buffer.h"

//#define SS_OPTIONS 2

static void advance_lfsr(simon_says_gamestate_t* gs)
{
    uint32_t bit = gs->lfsr & (uint32_t)1;
    gs->lfsr >>= 1;
    if(bit) {
        gs->lfsr ^= ((1ul << 31) | (1ul << 29) | (1ul << 25) | (1ul << 24));
    }
}

void simon_says_init(simon_says_gamestate_t* gs,
                     uint32_t lfsr_seed)
{
    gs->lfsr_seed = lfsr_seed;
    gs->lfsr = lfsr_seed;

    gs->t_now = 0;
    gs->t_last_press = -1;

    gs->level_num = 1;
    gs->step_num = 0;
    gs->player_active = PLAYER_ACTIVE_GENERATOR_ACTIVE;

    gs->game_lost = 0;

    gs->output_high_time = 5;
    gs->output = 0;
}

/**
 * Advances the internal state of the simon says game by one step
 *
 * Should be called on a successful user recall or when a sequence number has been generated
 */
static void step_simon_one(simon_says_gamestate_t* gs)
{
    gs->step_num++;
    if (gs->step_num == gs->level_num) {
        gs->lfsr = gs->lfsr_seed;

        gs->step_num = 0;

        switch (gs->player_active) {
            case PLAYER_ACTIVE_GENERATOR_ACTIVE: {
                gs->player_active = PLAYER_ACTIVE_PLAYER_ACTIVE;
                break;
            }

            case PLAYER_ACTIVE_PLAYER_ACTIVE: {
                gs->level_num++;
                gs->player_active = PLAYER_ACTIVE_GENERATOR_ACTIVE;
                break;
            }
        }
    } else {
        // really should be "i < clog2(SS_OPTIONS)", but '4' should cover it.
        for (int i = 0; i < 4; i++) { advance_lfsr(gs); }
    }
}

/**
 *
 */
static uint8_t peek_next_sequence_number(const simon_says_gamestate_t* gs)
{
    return (gs->lfsr % SS_OPTIONS);
}

static inline uint8_t uint8_to_hexdigit(uint8_t n)
{
    return (n < 10) ? (n + '0') : (n + ('a' - 10));
}

static void simon_says_print_state(const simon_says_gamestate_t* gs)
{
    saeclib_u8_circular_buffer_pushone(uart_buffer, '\r');
    saeclib_u8_circular_buffer_pushone(uart_buffer, '\n');

    // todo: impl saeclib_u8_circular_buffer_popone()?
    char s[9];
    s[0] = 'l'; s[1] = ':'; s[2] = uint8_to_hexdigit(gs->level_num);
    s[3] = ' ';
    s[4] = 's'; s[5] = ':'; s[6] = uint8_to_hexdigit(gs->step_num);
    s[7] = ' ';
    s[8] = (gs->player_active == PLAYER_ACTIVE_GENERATOR_ACTIVE) ? 'g':'p';
    for (int i = 0; i < sizeof(s); saeclib_u8_circular_buffer_pushone(uart_buffer, s[i++]));

}

void simon_says_advance_gamestate(simon_says_gamestate_t* gs,
                                  simon_says_user_input_t ui,
                                  uint16_t dt_ms)
{
    gs->t_now += dt_ms;

    // TODO: check for timeout
    simon_says_print_state(gs);

    //
    switch (gs->player_active) {
        case PLAYER_ACTIVE_GENERATOR_ACTIVE: {
            const uint8_t expected = SS_OPTIONS;
            if (ui == (1 << expected)) {
                gs->t_last_press = gs->t_now;
                gs->output = (1 << (peek_next_sequence_number(gs)));
                step_simon_one(gs);
            } else if (ui) {
                // if the player pressed any other key besides the "generate" button during the
                // generation period, they forgot how long the game was! they lose.
                gs->game_lost = 1;
                gs->t_last_press = gs->t_now;
                gs->output = (1 << 7);
            } else {
                // do nothing
            }

            break;
        }

        case PLAYER_ACTIVE_PLAYER_ACTIVE: {
            const uint8_t expected = peek_next_sequence_number(gs);
            saeclib_u8_circular_buffer_pushone(uart_buffer, uint8_to_hexdigit(expected));
            if (ui == (1 << expected)) {
                // user pressed the right button
                gs->t_last_press = gs->t_now;
                gs->output = (1 << expected);
                step_simon_one(gs);
            } else if (ui) {
                // user pressed the wrong button
                gs->game_lost = 1;
                gs->t_last_press = gs->t_now;
                gs->output = (1 << 7);
            } else {
                // no button press, do nothing.
            }

            break;
        }
    }

    // clear output if output high time exceeded
    if ((gs->t_now - gs->t_last_press) > gs->output_high_time){
        gs->output &= ~((1 << SS_OPTIONS) - 1);
    }
}
