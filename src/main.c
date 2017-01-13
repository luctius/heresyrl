/*
    This file is part of heresyRL.

    heresyRL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    heresyRL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with heresyRL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include "uncursed.h"

#include "heresyrl_def.h"
#include "cmdline.h"
#include "options.h"
#include "logging.h"
#include "game.h"
#include "random.h"
#include "coord.h"
#include "inventory.h"
#include "input.h"
#include "turn_tick.h"
#include "ui/ui.h"
#include "items/items.h"
#include "monster/monster.h"
#include "dungeon/dungeon_map.h"
#include "wizard/wizard_mode.h"
#include "status_effects/status_effects.h"
#include "careers/careers.h"

static void sigfunc(int s) {
    FIX_UNUSED(s);
}

static void sigfunc_quit(int s) {
    FIX_UNUSED(s);

    gbl_game->running = false;
}

int main(int argc, char *argv[]) {
    struct gengetopt_args_info args_info;
    int debug_lvl = LG_DEBUG_LEVEL_GAME_INFO;
    int log_size = 100;

    srand(time(NULL));

    initialize_uncursed(&argc, argv);
    assert(atexit(exit_uncursed) == 0);

    if (cmdline_parser (argc, argv, &args_info) != 0) exit(EXIT_FAILURE);
    opt_parse_options(&args_info);
    cmdline_parser_free(&args_info);
    assert(atexit(opt_exit) == 0);

    if (options.debug) {
        debug_lvl = LG_DEBUG_LEVEL_DEBUG;
        log_size = 10000;
    }

    lg_init(options.log_file_name, debug_lvl, log_size);
    assert(atexit(lg_exit) == 0);

    game_init(NULL, rand());
    assert(atexit(game_exit) == 0);

    if (options.print_map_only) {
        struct dm_spawn_settings spwn_sett = {
            .size = cd_create(100,100),
            .threat_lvl_min  = 0,
            .threat_lvl_max = 0,
            .item_chance = 0,
            .monster_chance = 0,
            .seed = random_int32(gbl_game->random),
            .type = DUNGEON_TYPE_ALL,
        };

        dm_generate_map(&spwn_sett);
        exit(EXIT_SUCCESS);
    }

    itmlst_items_list_init();
    assert(atexit(itmlst_items_list_exit) == 0);

    se_init();
    assert(atexit(se_exit) == 0);

    msrlst_monster_list_init();
    assert(atexit(msrlst_monster_list_exit) == 0);

    tt_init();
    assert(atexit(tt_exit) == 0);

    ge_init();
    assert(atexit(ge_exit) == 0);

    cr_init();
    assert(atexit(cr_exit) == 0);

    inp_init();
    assert(atexit(inp_exit) == 0);

    int cols, lines;
    initscr(); //  Start curses mode
    start_color();
    getmaxyx(stdscr, lines, cols);
    ui_create(cols, lines);
    assert(atexit(ui_destroy) == 0);

    if (options.wz_mode) {
        wz_init();
        assert(atexit(wz_exit) == 0);
    }

    bool valid_player = false;
    if (options.play_recording == false) {
        if (game_load() == false) {
            if (options.test_auto) {
                System_msg("Loading game failed.");
                exit(EXIT_FAILURE);
            }

            lg_init(options.log_file_name, debug_lvl, log_size);
            game_init(NULL, rand());
            System_msg("Loading game failed.");
        }
        else if (gbl_game->player_data.player != NULL) {
            valid_player = true;
        }
    }

    /* char creation */
    if (valid_player == false) {
        if (char_creation_window() ) valid_player = true;
        else exit(EXIT_FAILURE);
    }

    if (valid_player) {
        game_init_map();
        assert(atexit(game_preexit) == 0);

        update_screen();
        charwin_refresh();

        /*initialise signal handler*/
        struct sigaction setmask;
        sigemptyset( &setmask.sa_mask );
        setmask.sa_handler = sigfunc_quit;
        setmask.sa_flags   = 0;
        sigaction( SIGHUP,  &setmask, (struct sigaction *) NULL );      /* Hangup */
        sigaction( SIGINT,  &setmask, (struct sigaction *) NULL );      /* Interrupt (Ctrl-C) */
        setmask.sa_handler = sigfunc;
        sigaction( SIGQUIT, &setmask, (struct sigaction *) NULL );      /* Quit (Ctrl-\) */

        while(gbl_game->running == true) {
            tt_process(gbl_game->current_map);
            if (gbl_game->running) {
                game_new_tick();

                getmaxyx(stdscr, lines, cols);
                ui_create(cols, lines);

                if (gbl_game->player_data.exit_map) {
                    game_cleanup();
                    game_init_map();
                }
            }
        }
    }
    else lg_error("Player invalid.");
    System_msg("Goodbye.");

    usleep(500000);

    printf("Done.\n");

    return EXIT_SUCCESS;
}
