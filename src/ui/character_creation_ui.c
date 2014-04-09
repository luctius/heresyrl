#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <ncurses.h>
#include <sys/param.h>
#include <string.h>

#include "ui.h"
#include "ui_common.h"

#include "options.h"

#include "logging.h"
#include "player.h"
#include "inventory.h"
#include "input.h"
#include "game.h"
#include "monster/monster.h"
#include "monster/monster_static.h"

static int homeworld_id[] = { MSR_ID_BASIC_FERAL, MSR_ID_BASIC_HIVE, };
static char *homeworld_desc[] = { "Feral", "Hive", };


bool char_creation_window(void) {
    werase(map_win->win);
    werase(char_win->win);
    wrefresh(map_win->win);
    wrefresh(char_win->win);

    struct pl_player *plr = &gbl_game->player_data;
    plr->player = msr_create(0);
    plr->player->unique_name = "";
    charwin_refresh();

    struct msr_monster *player = plr->player;

    const char *enter_name_string = "Please enter your name: ";
    mvwprintw(map_win->win, 1, 1, enter_name_string);

    int name_buffer_sz = 20;
    int name_buffer_idx = 0;
    char name_buffer[name_buffer_sz];
    memset(name_buffer, 0x0, name_buffer_sz * sizeof(char) );
    enum inp_keys k;
    bool name_done = false;

    while (name_done == false) {
        wrefresh(map_win->win);

        k = inp_get_input_text(gbl_game->input);
        switch (k) {
            case '\n': name_done = true; break;
            case 24: return false;
            case KEY_BACKSPACE: 
                if (name_buffer_idx >= 0) {
                    name_buffer[name_buffer_idx--] = '\0'; 
                    name_buffer[name_buffer_idx] = '\0'; 
                }
                break;
            default:
                if (name_buffer_idx < (name_buffer_sz-2) ) {
                    name_buffer[name_buffer_idx++] = (char) k;
                }
        }

        wmove(map_win->win, 1, strlen(enter_name_string) +2 );
        wclrtoeol(map_win->win);
        mvwprintw(map_win->win, 1, strlen(enter_name_string) +2, name_buffer);
    }

    player->unique_name = malloc( (name_buffer_idx +5) * sizeof(char) );
    memcpy(player->unique_name, name_buffer, name_buffer_idx+1);
    charwin_refresh();



    werase(map_win->win);
    struct msr_monster *homeworld[ARRAY_SZ(homeworld_id)];
    for (unsigned int i = 0; i < ARRAY_SZ(homeworld_id); i++) {
        homeworld[i] = msr_create(homeworld_id[i]);
    }

    bool race_done = false;
    while (race_done == false) {
        mvwprintw(map_win->win, 1, 1, "Choose your homeworld");
        for (unsigned int i = 0; i < ARRAY_SZ(homeworld_id); i++) {
            char *hw_desc = homeworld_desc[i];
            mvwprintw(map_win->win, 4 + i, 1, "%c) %s", inp_key_translate_idx(i), hw_desc);
        }
        mvwprintw(map_win->win, map_win->lines -2, 1, "[U] choose  [x] examine");
        mvwprintw(map_win->win, map_win->lines -1, 1, "[q] quit");

        wrefresh(map_win->win);
        k = inp_get_input(gbl_game->input);

        int sel_idx = -1;
        switch (k) {
            case INP_KEY_QUIT: 
            case INP_KEY_ESCAPE: return false; break;
            case INP_KEY_UP_RIGHT: 
            case INP_KEY_USE: 
                mvwprintw(map_win->win, map_win->lines -3, 1, "Choose which homeworld?");
                wrefresh(map_win->win);
                werase(map_win->win);

                sel_idx = inp_get_input_idx(gbl_game->input);
                break;

            case INP_KEY_EXAMINE: 
                mvwprintw(map_win->win, map_win->lines -3, 1, "Examine which homeworld?");
                wrefresh(map_win->win);
                werase(map_win->win);

                sel_idx = inp_get_input_idx(gbl_game->input);
                if (sel_idx < ARRAY_SZ(homeworld_id) ) {
                    textwin_init(map_win, 1, 8 + ARRAY_SZ(homeworld_id), 0, 0);
                    textwin_add_text(map_win,homeworld[sel_idx]->description );
                    textwin_display_text(map_win);
                }
                sel_idx = -1;
                break;
            default: break;
        }

        if ( (sel_idx >= 0) && (sel_idx < ARRAY_SZ(homeworld_id) ) ) {
            /* copy name*/
            char *name = player->unique_name;
            player->unique_name = NULL;

            /* create player */
            plr_create(plr, name, homeworld_id[sel_idx], MSR_GENDER_MALE);
            charwin_refresh();
            race_done = true;
        }
    }

    for (unsigned int i = 0; i < ARRAY_SZ(homeworld_id); i++) {
        msr_destroy(homeworld[i], NULL);
    }

    return true;
}

