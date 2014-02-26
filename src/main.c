#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>
#include <time.h>

#include "heresyrl_def.h"
#include "logging.h"
#include "map_display.h"
#include "monster.h"
#include "simple_dungeon.h"

struct hrl_window *map_win = NULL;
struct hrl_window *char_win = NULL;
struct hrl_window *msg_win = NULL;

int main(int argc, char *argv[])
{
    int ch;
    int x = 120;
    int y = 100;
    struct sd_map *map = NULL;
    int xpos = 0;
    int ypos = 0;
    struct msr_monster *player = NULL;

 	srand(time(NULL));
    gbl_log = lg_init(LG_DEBUG_LEVEL_DEBUG, 100);
    msr_monster_list_init();

    map = sd_alloc_map(x,y);
    sd_generate_map(map);

    player = msr_create();
    player->icon = '@';
    player->colour = DPL_COLOUR_NORMAL;

    if (sd_tile_instance(map, TILE_TYPE_STAIRS_UP, 0, &xpos, &ypos) == false) exit(1);
    if (msr_insert_monster(player, map, xpos, ypos) == false) exit(1);

    initscr(); /*  Start curses mode         */
    refresh(); /*  Print it on to the real screen */
    create_ui(COLS, LINES, &map_win, &char_win, &msg_win);

    cbreak();
    noecho();
    //timeout(1);
    keypad(stdscr, TRUE);

    do {
        int new_xpos = xpos;
        int new_ypos = ypos;
        if (ch == KEY_UP) { new_ypos--; }
        if (ch == KEY_RIGHT) { new_xpos++; }
        if (ch == KEY_DOWN) { new_ypos++; }
        if (ch == KEY_LEFT) { new_xpos--; }

        if (msr_move_monster(player, map, new_xpos, new_ypos) == true) {
            xpos = new_xpos;
            ypos = new_ypos;
        }

        create_ui(COLS, LINES, &map_win, &char_win, &msg_win);
        win_display_map(map_win, map, xpos, ypos);
    }
    while((ch = getch()) != 27 && ch != 'q');

    destroy_ui(map_win, char_win, msg_win);
    map_win = char_win = msg_win = NULL;
    lg_set_callback(gbl_log, NULL, NULL);
    refresh();          /*  Print it on to the real screen */
    endwin();           /*  End curses mode       */

    lg_printf("Goodbye :)");
    lg_exit(gbl_log);
    sd_free_map(map);
    msr_monster_list_exit();

    return 0;
}
