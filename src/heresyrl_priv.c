#include <ncurses.h>
#include <assert.h>

#include "heresyrl_def.h"

extern inline int pyth(int side1, int side2);

enum colours {
    CLR_PAIR_WHITE,
    CLR_PAIR_RED,
    CLR_PAIR_GREEN,
    CLR_PAIR_YELLOW,
    CLR_PAIR_BLUE,
    CLR_PAIR_MAGENTA,
    CLR_PAIR_CYAN,
    CLR_PAIR_BLACK,

    CLR_PAIR_BG_WHITE,
    CLR_PAIR_BG_RED,
    CLR_PAIR_BG_GREEN,
    CLR_PAIR_BG_YELLOW,
    CLR_PAIR_BG_BLUE,
    CLR_PAIR_BG_MAGENTA,
    CLR_PAIR_BG_CYAN,
};

static int term_colour_table[TERM_COLOUR_MAX];

void generate_colours(void) {
        /* Prepare the color pairs */
        init_pair(CLR_PAIR_WHITE   , COLOR_WHITE   , COLOR_BLACK);
        init_pair(CLR_PAIR_RED     , COLOR_RED     , COLOR_BLACK);
        init_pair(CLR_PAIR_GREEN   , COLOR_GREEN   , COLOR_BLACK);
        init_pair(CLR_PAIR_YELLOW  , COLOR_YELLOW  , COLOR_BLACK);
        init_pair(CLR_PAIR_BLUE    , COLOR_BLUE    , COLOR_BLACK);
        init_pair(CLR_PAIR_MAGENTA , COLOR_MAGENTA , COLOR_BLACK);
        init_pair(CLR_PAIR_CYAN    , COLOR_CYAN    , COLOR_BLACK);
        init_pair(CLR_PAIR_BLACK   , COLOR_BLACK   , COLOR_BLACK);

        init_pair(CLR_PAIR_BG_WHITE   , COLOR_BLACK   , COLOR_WHITE);
        init_pair(CLR_PAIR_BG_RED     , COLOR_BLACK     , COLOR_RED);
        init_pair(CLR_PAIR_BG_GREEN   , COLOR_BLACK   , COLOR_GREEN);
        init_pair(CLR_PAIR_BG_YELLOW  , COLOR_BLACK  , COLOR_YELLOW);
        init_pair(CLR_PAIR_BG_BLUE    , COLOR_BLACK    , COLOR_BLUE);
        init_pair(CLR_PAIR_BG_MAGENTA , COLOR_BLACK , COLOR_MAGENTA);
        init_pair(CLR_PAIR_BG_CYAN    , COLOR_BLACK    , COLOR_CYAN);

        /* Prepare the colors */
        term_colour_table[TERM_COLOUR_DARK]     = (COLOR_PAIR(CLR_PAIR_BLACK));
        term_colour_table[TERM_COLOUR_WHITE]    = (COLOR_PAIR(CLR_PAIR_WHITE) | A_BOLD);
        term_colour_table[TERM_COLOUR_SLATE]    = (COLOR_PAIR(CLR_PAIR_WHITE));
        term_colour_table[TERM_COLOUR_ORANGE]   = (COLOR_PAIR(CLR_PAIR_YELLOW) | A_BOLD);
        term_colour_table[TERM_COLOUR_RED]      = (COLOR_PAIR(CLR_PAIR_RED));
        term_colour_table[TERM_COLOUR_GREEN]    = (COLOR_PAIR(CLR_PAIR_GREEN));
        term_colour_table[TERM_COLOUR_BLUE]     = (COLOR_PAIR(CLR_PAIR_BLUE));
        term_colour_table[TERM_COLOUR_UMBER]    = (COLOR_PAIR(CLR_PAIR_YELLOW));
        term_colour_table[TERM_COLOUR_L_DARK]   = (COLOR_PAIR(CLR_PAIR_BLACK) | A_BOLD);
        term_colour_table[TERM_COLOUR_L_WHITE]  = (COLOR_PAIR(CLR_PAIR_WHITE));
        term_colour_table[TERM_COLOUR_L_PURPLE] = (COLOR_PAIR(CLR_PAIR_MAGENTA));
        term_colour_table[TERM_COLOUR_YELLOW]   = (COLOR_PAIR(CLR_PAIR_YELLOW) | A_BOLD);
        term_colour_table[TERM_COLOUR_L_RED]    = (COLOR_PAIR(CLR_PAIR_MAGENTA) | A_BOLD);
        term_colour_table[TERM_COLOUR_L_GREEN]  = (COLOR_PAIR(CLR_PAIR_GREEN) | A_BOLD);
        term_colour_table[TERM_COLOUR_L_BLUE]   = (COLOR_PAIR(CLR_PAIR_BLUE) | A_BOLD);
        term_colour_table[TERM_COLOUR_L_UMBER]  = (COLOR_PAIR(CLR_PAIR_YELLOW));

        term_colour_table[TERM_COLOUR_PURPLE]      = (COLOR_PAIR(CLR_PAIR_MAGENTA));
        term_colour_table[TERM_COLOUR_VIOLET]      = (COLOR_PAIR(CLR_PAIR_MAGENTA));
        term_colour_table[TERM_COLOUR_TEAL]        = (COLOR_PAIR(CLR_PAIR_CYAN));
        term_colour_table[TERM_COLOUR_MUD]         = (COLOR_PAIR(CLR_PAIR_YELLOW));
        term_colour_table[TERM_COLOUR_L_YELLOW]    = (COLOR_PAIR(CLR_PAIR_YELLOW | A_BOLD));
        term_colour_table[TERM_COLOUR_MAGENTA]     = (COLOR_PAIR(CLR_PAIR_MAGENTA | A_BOLD));
        term_colour_table[TERM_COLOUR_L_TEAL]      = (COLOR_PAIR(CLR_PAIR_CYAN | A_BOLD));
        term_colour_table[TERM_COLOUR_L_VIOLET]    = (COLOR_PAIR(CLR_PAIR_MAGENTA | A_BOLD));
        term_colour_table[TERM_COLOUR_L_PINK]      = (COLOR_PAIR(CLR_PAIR_MAGENTA | A_BOLD));
        term_colour_table[TERM_COLOUR_MUSTARD]     = (COLOR_PAIR(CLR_PAIR_YELLOW));
        term_colour_table[TERM_COLOUR_BLUE_SLATE]  = (COLOR_PAIR(CLR_PAIR_BLUE));
        term_colour_table[TERM_COLOUR_DEEP_L_BLUE] = (COLOR_PAIR(CLR_PAIR_BLUE));

        term_colour_table[TERM_COLOUR_BG_WHITE]    = (COLOR_PAIR(CLR_PAIR_BG_WHITE) | A_BOLD);
        term_colour_table[TERM_COLOUR_BG_SLATE]    = (COLOR_PAIR(CLR_PAIR_BG_WHITE));
        term_colour_table[TERM_COLOUR_BG_ORANGE]   = (COLOR_PAIR(CLR_PAIR_BG_YELLOW) | A_BOLD);
        term_colour_table[TERM_COLOUR_BG_RED]      = (COLOR_PAIR(CLR_PAIR_BG_RED));
        term_colour_table[TERM_COLOUR_BG_GREEN]    = (COLOR_PAIR(CLR_PAIR_BG_GREEN));
        term_colour_table[TERM_COLOUR_BG_BLUE]     = (COLOR_PAIR(CLR_PAIR_BG_BLUE));
        term_colour_table[TERM_COLOUR_BG_UMBER]    = (COLOR_PAIR(CLR_PAIR_BG_YELLOW));
        term_colour_table[TERM_COLOUR_BG_L_WHITE]  = (COLOR_PAIR(CLR_PAIR_BG_WHITE));
        term_colour_table[TERM_COLOUR_BG_L_PURPLE] = (COLOR_PAIR(CLR_PAIR_BG_MAGENTA));
        term_colour_table[TERM_COLOUR_BG_YELLOW]   = (COLOR_PAIR(CLR_PAIR_BG_YELLOW) | A_BOLD);
        term_colour_table[TERM_COLOUR_BG_L_RED]    = (COLOR_PAIR(CLR_PAIR_BG_MAGENTA) | A_BOLD);
        term_colour_table[TERM_COLOUR_BG_L_GREEN]  = (COLOR_PAIR(CLR_PAIR_BG_GREEN) | A_BOLD);
        term_colour_table[TERM_COLOUR_BG_L_BLUE]   = (COLOR_PAIR(CLR_PAIR_BG_BLUE) | A_BOLD);
        term_colour_table[TERM_COLOUR_BG_L_UMBER]  = (COLOR_PAIR(CLR_PAIR_BG_YELLOW));

        term_colour_table[TERM_COLOUR_BG_PURPLE]      = (COLOR_PAIR(CLR_PAIR_BG_MAGENTA));
        term_colour_table[TERM_COLOUR_BG_VIOLET]      = (COLOR_PAIR(CLR_PAIR_BG_MAGENTA));
        term_colour_table[TERM_COLOUR_BG_TEAL]        = (COLOR_PAIR(CLR_PAIR_BG_CYAN));
        term_colour_table[TERM_COLOUR_BG_MUD]         = (COLOR_PAIR(CLR_PAIR_BG_YELLOW));
        term_colour_table[TERM_COLOUR_BG_L_YELLOW]    = (COLOR_PAIR(CLR_PAIR_BG_YELLOW | A_BOLD));
        term_colour_table[TERM_COLOUR_BG_MAGENTA]     = (COLOR_PAIR(CLR_PAIR_BG_MAGENTA | A_BOLD));
        term_colour_table[TERM_COLOUR_BG_L_TEAL]      = (COLOR_PAIR(CLR_PAIR_BG_CYAN | A_BOLD));
        term_colour_table[TERM_COLOUR_BG_L_VIOLET]    = (COLOR_PAIR(CLR_PAIR_BG_MAGENTA | A_BOLD));
        term_colour_table[TERM_COLOUR_BG_L_PINK]      = (COLOR_PAIR(CLR_PAIR_BG_MAGENTA | A_BOLD));
        term_colour_table[TERM_COLOUR_BG_MUSTARD]     = (COLOR_PAIR(CLR_PAIR_BG_YELLOW));
        term_colour_table[TERM_COLOUR_BG_BLUE_SLATE]  = (COLOR_PAIR(CLR_PAIR_BG_BLUE));
        term_colour_table[TERM_COLOUR_BG_DEEP_L_BLUE] = (COLOR_PAIR(CLR_PAIR_BG_BLUE));
}

 int get_colour(int cc) {
    assert(cc >= 0);
    assert(cc < TERM_COLOUR_MAX);
    //lg_debug("return color[%d]=%d", cc, term_colour_table[cc]);
    return term_colour_table[cc];
}
