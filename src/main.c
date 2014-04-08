#define CMDLINE_PARSER_PACKAGE "heresyrl"
#define CMDLINE_PARSER_VERSION VERSION

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>


#include "heresyrl_def.h"
#include "cmdline.h"
#include "options.h"
#include "logging.h"
#include "game.h"
#include "coord.h"
#include "tiles.h"
#include "inventory.h"
#include "input.h"
#include "turn_tick.h"
#include "ui/ui.h"
#include "items/items.h"
#include "monster/monster.h"
#include "dungeon/dungeon_map.h"

static void sigfunc(int s) {
    FIX_UNUSED(s);

    System_msg("Please press Ctrl-X to quit\n");
}

int main(int argc, char *argv[]) {
    struct gengetopt_args_info args_info;

    if (cmdline_parser (argc, argv, &args_info) != 0) exit(1);

    opt_parse_options(&args_info);

    if (options.debug) {
        gbl_log = lg_init(options.log_file_name, LG_DEBUG_LEVEL_DEBUG, 10000);
    }
    else {
        gbl_log = lg_init(options.log_file_name, LG_DEBUG_LEVEL_INFORMATIONAL, 10000);
    }

 	srand(time(NULL));
    game_init(NULL, rand());

    initscr(); //  Start curses mode
    if (has_colors() == FALSE) exit(1);
    if (start_color() == ERR) exit(1);
    refresh(); //  Print it on to the real screen

    int cols, lines;
    getmaxyx(stdscr, lines, cols);
    ui_create(cols, lines);

    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    game_load();
    game_init_map();

    coord_t *player_pos = &gbl_game->player_data.player->pos;
    mapwin_display_map(gbl_game->current_map, player_pos);
    charwin_refresh();

    /*initialise signal handler*/
    struct sigaction setmask;
    sigemptyset( &setmask.sa_mask );
    setmask.sa_handler = sigfunc;
    setmask.sa_flags   = 0;
    sigaction( SIGHUP,  &setmask, (struct sigaction *) NULL );      /* Hangup */
    sigaction( SIGINT,  &setmask, (struct sigaction *) NULL );      /* Interrupt (Ctrl-C) */

    while(gbl_game->running == true) {
        tt_process(gbl_game->current_map);
        if (gbl_game->running) game_new_tick();

        getmaxyx(stdscr, lines, cols);
        ui_create(cols, lines);
    }
    GM_msg("Goodbye.");
    usleep(500000);

    game_exit();
    ui_destroy();

    //clear();
    refresh();          //  Print it on to the real screen
    endwin();           //  End curses mode

    lg_exit(gbl_log);
    return 0;
}
