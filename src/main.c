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

    if (options.play_recording == true) {
        gbl_game->running = false;
    }
    else System_msg("Please press Ctrl-X to quit");
}

int main(int argc, char *argv[]) {
    struct gengetopt_args_info args_info;

    if (cmdline_parser (argc, argv, &args_info) != 0) exit(EXIT_FAILURE);
    opt_parse_options(&args_info);
    cmdline_parser_free(&args_info);

    int debug_lvl = LG_DEBUG_LEVEL_GAME_INFO;
    int log_size = 100;
    if (options.debug) {
        debug_lvl = LG_DEBUG_LEVEL_DEBUG;
        log_size = 10000;
    }
    gbl_log = lg_init(options.log_file_name, debug_lvl, log_size);

 	srand(time(NULL));
    game_init(NULL, rand());

    initscr(); //  Start curses mode
    if (has_colors() == FALSE) exit(EXIT_FAILURE);
    if (start_color() == ERR) exit(EXIT_FAILURE);
    refresh(); //  Print it on to the real screen

    int cols, lines;
    getmaxyx(stdscr, lines, cols);
    ui_create(cols, lines);

    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    bool valid_player = false;
    if (game_load() && options.play_recording == false) {
        if (gbl_game->player_data.player != NULL) valid_player = true;
        else {
            game_exit();
            lg_exit(gbl_log);
            gbl_log = lg_init(options.log_file_name, debug_lvl, log_size);
            game_init(NULL, rand());
            System_msg("Loading game failed.");
        }
    }

        /* char creation */
    if (valid_player == false) {
        if (char_creation_window() ) valid_player = true;
    }

    if (valid_player) {
        game_init_map();

        update_screen();
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
    }

    System_msg("Goodbye.");
    usleep(500000);

    game_exit();
    ui_destroy();
    lg_exit(gbl_log);

    clear();
    refresh();          //  Print it on to the real screen
    endwin();           //  End curses mode

    printf("Done.\n");

    return EXIT_SUCCESS;
}
