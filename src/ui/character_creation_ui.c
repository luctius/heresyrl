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
#include "dungeon/spawn.h"
#include "monster/monster.h"
#include "monster/monster_static.h"

struct homeworld_id {
    int id;
    const char *desc;
    const char *long_desc;
};

static struct homeworld_id homeworld_id[] =  { 
    { .id = MID_BASIC_FERAL, .desc = "Feral", .long_desc = "...", }, 
    { .id = MID_BASIC_HIVE,  .desc = "Hive",  .long_desc = "...", }, 
};

struct spwn_monster_item items[] = {  
    {.id=IID_FLAK_JACKET,    .min=1,.max=1,.wear=true}, 
    {.id=IID_AUTO_GUN,       .min=1,.max=1,.wear=true}, 
    {.id=IID_BASIC_AMMO_SP,  .min=100,.max=500,.wear=false},
    {.id=IID_GLOW_GLOBE,     .min=1,.max=1,.wear=false},
    {.id=IID_FIRE_BOMB,      .min=1,.max=3,.wear=false},
    {.id=IID_THROWING_KNIFE, .min=1,.max=3,.wear=false},
    {.id=IID_STIMM_HEALTH,   .min=2,.max=4,.wear=false},
    {.id=IID_STIMM_DEATH,    .min=5,.max=5,.wear=false},
    {0,0,0,0,} };


bool char_creation_window(void) {
    werase(map_win->win);
    werase(char_win->win);
    wrefresh(map_win->win);
    wrefresh(char_win->win);

    struct pl_player *plr = &gbl_game->player_data;
    plr->player = msr_create(MID_DUMMY);

    struct msr_monster *player = plr->player;
    player->unique_name = "";
    charwin_refresh();

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
            case '\n': if (strlen(name_buffer) > 0 ) { name_done = true; } break;
            case 24: return false;
            case INP_KEY_BACKSPACE: 
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

    bool race_done = false;
    while (race_done == false) {
        mvwprintw(map_win->win, 1, 1, "Choose your homeworld");
        for (unsigned int i = 0; i < ARRAY_SZ(homeworld_id); i++) {
            const char *hw_desc = homeworld_id[i].desc;
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
                if (sel_idx < (int) ARRAY_SZ(homeworld_id) ) {
                    ui_printf(map_win,"%s", homeworld_id[sel_idx].long_desc);
                    wrefresh(map_win->win);
                }
                sel_idx = -1;
                break;
            default: break;
        }

        if ( (sel_idx >= 0) && (sel_idx < (int) ARRAY_SZ(homeworld_id) ) ) {
            /* copy name*/
            char *name = player->unique_name;
            player->unique_name = NULL;

            /* create player */
            plr_create(plr, name, homeworld_id[sel_idx].id, MSR_GENDER_MALE);
            charwin_refresh();
            race_done = true;
        }
    }

    /* give newly born his items */
    int i = 0;
    while (items[i].max != 0) {
        spwn_add_item_to_monster(plr->player, &items[i], gbl_game->random);
        i++;
    }

    return true;
}

