#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>
#include <time.h>
#include <signal.h>

#include "heresyrl_def.h"
#include "cmdline.h"
#include "ui.h"
#include "dungeon_creator.h"
#include "logging.h"
#include "monster.h"
#include "game.h"
#include "items.h"
#include "coord.h"
#include "tiles.h"
#include "inventory.h"
#include "input.h"
#include "monster_action.h"

static void sigfunc(int signal) {
    FIX_UNUSED(signal);

    gbl_game->running = false;
}

int main(int argc, char *argv[]) {
    struct gengetopt_args_info args_info;

    if (cmdline_parser (argc, argv, &args_info) != 0) exit(1);

    if (args_info.debug_flag) {
        gbl_log = lg_init(args_info.log_file_arg, LG_DEBUG_LEVEL_DEBUG, 10000);
    }
    else {
        gbl_log = lg_init(args_info.log_file_arg, LG_DEBUG_LEVEL_GAME, 10000);
    }

 	srand(time(NULL));
    game_init(NULL, rand());
    gbl_game->args_info = &args_info;

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
        ma_process();
        if (gbl_game->running) game_new_tick();

        getmaxyx(stdscr, lines, cols);
        ui_create(cols, lines);
    }

    game_exit();

    lg_print("Goodbye :)");
    getch();
    usleep(1000000);

    ui_destroy();

    clear();
    refresh();          //  Print it on to the real screen
    endwin();           //  End curses mode

    lg_exit(gbl_log);
    return 0;
}
