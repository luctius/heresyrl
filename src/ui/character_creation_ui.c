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

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/param.h>
#include <string.h>
#include <uncursed.h>

#include "ui.h"
#include "ui_common.h"

#include "options.h"

#include "logging.h"
#include "player.h"
#include "inventory.h"
#include "input.h"
#include "game.h"
#include "random.h"
#include "dungeon/spawn.h"
#include "monster/monster.h"
#include "careers/careers.h"

extern struct msr_monster static_monster_list[];

struct spwn_monster_item items[] = {
    {.id=IID_GLOW_GLOBE,          .min=1,.max=1,.wear=false},
    {.id=IID_STIMM_HEALTH_MINOR,   .min=0,.max=2,.wear=false},
    {.id=IID_STIMM_DEATH,   .min=5,.max=5,.wear=false},
    {.id=IID_MONEY,         .min=5,.max=20,.wear=false},
    {0,0,0,0,} };


bool char_creation_window(void) {
    werase(map_win->win);
    werase(char_win->win);
    wrefresh(map_win->win);
    wrefresh(char_win->win);

    enum homeworld_ids h_tid;
    enum background_ids b_tid;
    enum role_ids r_tid;

    struct pl_player *plr = &gbl_game->player_data;
    plr->player = msr_create(MID_PLAYER);
    cr_init_career(plr, CR_HWID_NONE, CR_BCKGRNDID_NONE, CR_ROLEID_NONE);

    struct msr_monster *player = plr->player;
    player->unique_name = L"";
    charwin_refresh();

    const char *enter_name_string = "Please enter your name:";

    System_msg("Welcome to heresyrl.");

    ui_print_reset(map_win);
    ui_printf(map_win, "%s ", enter_name_string);

    int name_buffer_sz = 200;
    int name_buffer_idx = 0;
    wchar_t name_buffer[name_buffer_sz];
    memset(name_buffer, 0x0, name_buffer_sz * sizeof(wchar_t) );
    enum inp_keys k;

    bool name_done = false;
    bool homeworld_done = false;
    bool background_done = false;
    bool role_done = false;

    if (options.char_name != NULL) {
        wcscpy(name_buffer, options.char_name);
        name_done = true;

        for (wchar_t *c = name_buffer; *c != 0; c++) {
            inp_add_to_log(gbl_game->input, *c);
        }
        inp_add_to_log(gbl_game->input, '\n');
    }
    if ( (options.char_hw != CR_HWID_NONE) ) {
        homeworld_done = true;
        h_tid = options.char_hw;

        inp_add_to_log(gbl_game->input, INP_KEY_APPLY);
        inp_add_to_log(gbl_game->input, (h_tid -1) );
    }
    if ( (options.char_bg != CR_BCKGRNDID_NONE) ) {
        background_done = true;
        b_tid = options.char_bg;

        inp_add_to_log(gbl_game->input, INP_KEY_APPLY);
        inp_add_to_log(gbl_game->input, (b_tid -1) );
    }
    if ( (options.char_role != CR_ROLEID_NONE) ) {
        role_done = true;
        r_tid = options.char_role;

        inp_add_to_log(gbl_game->input, INP_KEY_APPLY);
        inp_add_to_log(gbl_game->input, (r_tid -1) );
    }

    while (name_done == false) {
        wrefresh(map_win->win);

        k = inp_get_input_text(gbl_game->input);
        switch (k) {
            case INP_KEY_YES: if (wcslen(name_buffer) > 0 ) { name_done = true; } break;
            case INP_KEY_BACKSPACE:
                if (name_buffer_idx > 0) {
                    name_buffer[name_buffer_idx -1] = name_buffer[name_buffer_idx] = L'\0';
                    name_buffer_idx -= 1;
                }
                werase(map_win->win);
                ui_print_reset(map_win);
                ui_printf(map_win, "%s ", enter_name_string);
                break;
            default:
                if (name_buffer_idx < (name_buffer_sz-2) ) {
                    name_buffer[name_buffer_idx++] = (wchar_t) k;
                }
        }

        ui_print_reset(map_win);
        ui_printf(map_win, "%s %ls", enter_name_string, name_buffer);
    }

    player->unique_name = wcsdup(name_buffer);
    charwin_refresh();

    ui_print_reset(map_win);
    werase(map_win->win);

    GM_msg("Please select a race by first pressing 'a' and then the letter before the race you want.");
    GM_msg("This mechanism is used througout the game; first the action than the noun.");
    GM_msg("With 'x' you can examine more about the races.");
    GM_msg("Press '?' to view the help screen.");

    while (!homeworld_done) {
        ui_print_reset(map_win);

        int valid_choice = 0;
        ui_printf(map_win, "    Choose your Home World\n");
        for (unsigned int i = CR_HWID_NONE+1; i < CR_HWID_MAX; i++) {
            ui_printf(map_win, "%lc) %ls\n", inp_key_translate_idx(valid_choice++), cr_get_homeworld_by_id(i)->name);
        }
        ui_printf_ext(map_win, map_win->lines -2, 1, cs_ATTR "[a]" cs_CLOSE " acquire,  " cs_ATTR "[x]" cs_CLOSE " examine");
        ui_printf_ext(map_win, map_win->lines -1, 1, cs_ATTR "[q]" cs_CLOSE " continue  " cs_ATTR "[?]" cs_CLOSE " help");

        wrefresh(map_win->win);
        k = inp_get_input(gbl_game->input);

        int sel_idx = -1;
        switch (k) {
            case INP_KEY_NO:
            case INP_KEY_QUIT:
            case INP_KEY_ESCAPE: return false; /*break;*/
            case INP_KEY_ALL:
            case INP_KEY_APPLY:
                ui_printf_ext(map_win, map_win->lines -3, 1, "Choose which Home World?");
                wrefresh(map_win->win);
                werase(map_win->win);
                ui_print_reset(map_win);

                sel_idx = inp_get_input_idx(gbl_game->input) +1;
                if (sel_idx < (int) CR_HWID_MAX) {
                    if (sel_idx > CR_HWID_NONE) {
                        GM_msg(cs_PLAYER "You" cs_CLOSE " come from an %ls.", cr_get_homeworld_by_id(sel_idx)->name);
                    }
                }
                break;

            case INP_KEY_EXAMINE:
                ui_printf_ext(map_win, map_win->lines -3, 1, "Examine which Home World?");
                wrefresh(map_win->win);
                werase(map_win->win);

                sel_idx = inp_get_input_idx(gbl_game->input)+1;
                if (sel_idx < (int) MID_MAX) {
                    charwin_examine("Home World", cr_get_homeworld_by_id(sel_idx)->name, cr_get_homeworld_by_id(sel_idx)->description);
                }
                sel_idx = -1;
                break;
            case INP_KEY_HELP:
                help_window();
                charwin_refresh();
                break;
            default:
                werase(map_win->win);
                ui_print_reset(map_win);
                break;
        }

        if ( (sel_idx > CR_HWID_NONE) && (sel_idx < (int) CR_HWID_MAX) ) {
            h_tid = sel_idx;
            homeworld_done = true;
        }
    }

    while (!background_done) {
        ui_print_reset(map_win);

        int valid_choice = 0;
        ui_printf(map_win, "    Choose your Background\n");
        for (unsigned int i = CR_BCKGRNDID_NONE+1; i < CR_BCKGRNDID_MAX; i++) {
            ui_printf(map_win, "%lc) %ls\n", inp_key_translate_idx(valid_choice++), cr_get_background_by_id(i)->name);
        }
        ui_printf_ext(map_win, map_win->lines -2, 1, cs_ATTR "[a]" cs_CLOSE " acquire,  " cs_ATTR "[x]" cs_CLOSE " examine");
        ui_printf_ext(map_win, map_win->lines -1, 1, cs_ATTR "[q]" cs_CLOSE " continue  " cs_ATTR "[?]" cs_CLOSE " help");

        wrefresh(map_win->win);
        k = inp_get_input(gbl_game->input);

        int sel_idx = -1;
        switch (k) {
            case INP_KEY_NO:
            case INP_KEY_QUIT:
            case INP_KEY_ESCAPE: return false; /*break;*/
            case INP_KEY_ALL:
            case INP_KEY_APPLY:
                ui_printf_ext(map_win, map_win->lines -3, 1, "Choose which Background?");
                wrefresh(map_win->win);
                werase(map_win->win);
                ui_print_reset(map_win);

                sel_idx = inp_get_input_idx(gbl_game->input) +1;
                if (sel_idx < (int) CR_BCKGRNDID_MAX) {
                    if (sel_idx > CR_BCKGRNDID_NONE) {
                        GM_msg(cs_PLAYER "You" cs_CLOSE " were a %ls.", cr_get_background_by_id(sel_idx)->name);
                    }
                }
                break;

            case INP_KEY_EXAMINE:
                ui_printf_ext(map_win, map_win->lines -3, 1, "Examine which Background?");
                wrefresh(map_win->win);
                werase(map_win->win);

                sel_idx = inp_get_input_idx(gbl_game->input)+1;
                if (sel_idx < (int) MID_MAX) {
                    charwin_examine("Background", cr_get_background_by_id(sel_idx)->name, cr_get_background_by_id(sel_idx)->description);
                }
                sel_idx = -1;
                break;
            case INP_KEY_HELP:
                help_window();
                charwin_refresh();
                break;
            default:
                werase(map_win->win);
                ui_print_reset(map_win);
                break;
        }

        if ( (sel_idx > CR_BCKGRNDID_NONE) && (sel_idx < (int) CR_BCKGRNDID_MAX) ) {
            b_tid = sel_idx;
            background_done = true;
        }
    }

    while (!role_done) {
        ui_print_reset(map_win);

        int valid_choice = 0;
        ui_printf(map_win, "    Choose your Role\n");
        for (unsigned int i = CR_ROLEID_NONE+1; i < CR_ROLEID_MAX; i++) {
            ui_printf(map_win, "%lc) %ls\n", inp_key_translate_idx(valid_choice++), cr_get_role_by_id(i)->name);
        }
        ui_printf_ext(map_win, map_win->lines -2, 1, cs_ATTR "[a]" cs_CLOSE " acquire,  " cs_ATTR "[x]" cs_CLOSE " examine");
        ui_printf_ext(map_win, map_win->lines -1, 1, cs_ATTR "[q]" cs_CLOSE " continue  " cs_ATTR "[?]" cs_CLOSE " help");

        wrefresh(map_win->win);
        k = inp_get_input(gbl_game->input);

        int sel_idx = -1;
        switch (k) {
            case INP_KEY_NO:
            case INP_KEY_QUIT:
            case INP_KEY_ESCAPE: return false; /*break;*/
            case INP_KEY_ALL:
            case INP_KEY_APPLY:
                ui_printf_ext(map_win, map_win->lines -3, 1, "Choose which Role?");
                wrefresh(map_win->win);
                werase(map_win->win);
                ui_print_reset(map_win);

                sel_idx = inp_get_input_idx(gbl_game->input)+1;
                if (sel_idx < (int) CR_ROLEID_MAX) {
                    if (sel_idx > CR_ROLEID_NONE) {
                        GM_msg(cs_PLAYER "Your" cs_CLOSE " role is a %ls.", cr_get_role_by_id(sel_idx)->name);
                    }
                }
                break;

            case INP_KEY_EXAMINE:
                ui_printf_ext(map_win, map_win->lines -3, 1, "Examine which Role?");
                wrefresh(map_win->win);
                werase(map_win->win);

                sel_idx = inp_get_input_idx(gbl_game->input)+1;
                if (sel_idx < (int) MID_MAX) {
                    charwin_examine("Role", cr_get_role_by_id(sel_idx)->name, cr_get_role_by_id(sel_idx)->description);
                }
                sel_idx = -1;
                break;
            case INP_KEY_HELP:
                help_window();
                charwin_refresh();
                break;
            default:
                werase(map_win->win);
                ui_print_reset(map_win);
                break;
        }

        if ( (sel_idx > CR_ROLEID_NONE) && (sel_idx < (int) CR_ROLEID_MAX) ) {
            r_tid = sel_idx;
            role_done = true;
        }
    }

    cr_init_career(plr, h_tid, b_tid, r_tid);

    /* give newly born his items */
    int i = 0;
    while (items[i].max != 0) {
        spwn_add_item_to_monster(plr->player, &items[i], gbl_game->random);
        i++;
    }

    plr->loan = 500;
    plr->career.xp_current = 500;
    levelup_selection_window();

    System_msg("Welcome %ls.", player->unique_name);
    return true;
}

