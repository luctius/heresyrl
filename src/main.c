#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>
#include <time.h>

#include "heresyrl_def.h"
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


//int main(int argc, char *argv[])
int main(void)
{
    int ch;
    struct itm_item *item;
    coord_t pos = cd_create(0,0);

    gbl_log = lg_init(LG_DEBUG_LEVEL_DEBUG, 10000);
 	srand(time(NULL));
    game_init(NULL, rand());

    initscr(); //  Start curses mode
    if (has_colors() == FALSE) exit(1);
    if (start_color() == ERR) exit(1);
    refresh(); //  Print it on to the real screen

    ui_create(COLS, LINES);

    cbreak();
    noecho();
    //timeout(1);
    keypad(stdscr, TRUE);

    game_init_map();

    coord_t *player_pos = &gbl_game->player_data.player->pos;
    mapwin_display_map(gbl_game->current_map, player_pos);
    charwin_refresh(&gbl_game->player_data);

    while(gbl_game->running == true) {
        ma_process();
        if (gbl_game->running) game_new_tick();

        ui_create(COLS, LINES);
    }

    game_exit();
    ui_destroy();

    clear();
    refresh();          //  Print it on to the real screen
    endwin();           //  End curses mode

    lg_printf("Goodbye :)");
    lg_exit(gbl_log);
    return 0;
}
