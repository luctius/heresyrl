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

#include <inttypes.h>

#include "items/items.h"
#include "monster/monster.h"
#include "status_effects/status_effects.h"
#include "ai/pathfinding.h"
#include "dungeon/dungeon_map.h"
#include "dungeon/tiles.h"
#include "input.h"
#include "options.h"
#include "ui/ui.h"
#include "fov/sight.h"

static void cmd_fov_test(char *input) {
    struct msr_monster *player = gbl_game->player_data.player;
    struct dm_map *map = gbl_game->current_map;

    long int radius = 5;
    if (input != NULL) {
        radius = strtol(input, NULL, 10);
        if (radius == 0 || radius == LONG_MIN || radius ==  LONG_MAX) radius = 5;
    }

    coord_t p_pos = player->pos;
    coord_t e_pos = p_pos;
    int scr_x = get_viewport(p_pos.x, map_win->cols, map->sett.size.x);
    int scr_y = get_viewport(p_pos.y, map_win->lines, map->sett.size.y);

    int ch;
    bool examine_mode = true;
    while( ( (ch = inp_get_input(gbl_game->input)) != INP_KEY_ESCAPE) && (examine_mode == true) ) {
        switch (ch) {
            case INP_KEY_UP_LEFT:    e_pos.y--; e_pos.x--; break;
            case INP_KEY_UP:         e_pos.y--; break;
            case INP_KEY_UP_RIGHT:   e_pos.y--; e_pos.x++; break;
            case INP_KEY_RIGHT:      e_pos.x++; break;
            case INP_KEY_DOWN_RIGHT: e_pos.y++; e_pos.x++; break;
            case INP_KEY_DOWN:       e_pos.y++; break;
            case INP_KEY_DOWN_LEFT:  e_pos.y++; e_pos.x--; break;
            case INP_KEY_LEFT:       e_pos.x--; break;

            case INP_KEY_HELP:
            case INP_KEY_ESCAPE:
            case INP_KEY_QUIT:
            case INP_KEY_NO:
            case INP_KEY_YES:
            case INP_KEY_EXAMINE:    examine_mode = false; break;
            default: break;
        }
        if (examine_mode == false) break;

        if (e_pos.y < 0) e_pos.y = 0;
        if (e_pos.y >= map->sett.size.y) e_pos.y = map->sett.size.y -1;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= map->sett.size.x) e_pos.x = map->sett.size.x -1;

        chtype oldch = mvwinch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x);
        mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, (oldch & 0xFF) | get_colour(TERM_COLOUR_BG_RED) );
        wrefresh(map_win->win);
        mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, oldch);
    }

    coord_t *list;
    int len = sgt_explosion(map, &e_pos, 5, &list);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            struct dm_map_entity *me = dm_get_map_me(&list[i], map);
            if (me->test_var == 0) {
                me->test_var = TERM_COLOUR_UMBER;
            }
            else me->test_var++;
        }
        free(list);
    }
    update_screen();
}

static void cmd_refresh(char *input) {
    FIX_UNUSED(input);
    coord_t start = { .x = 0, .y = 0, };
    dm_clear_map_visibility(gbl_game->current_map, &start, &gbl_game->current_map->sett.size);

    sgt_calculate_all_light_sources(gbl_game->current_map);
    sgt_calculate_player_sight(gbl_game->current_map, gbl_game->player_data.player);
    update_screen();
}

static void cmd_show_map(char *input) {
    FIX_UNUSED(input);
    if (options.debug_show_map) {
        options.debug_show_map = false;
        options.debug_show_monsters = false;
    }
    else {
        options.debug_show_map = true;
        options.debug_show_monsters = true;
    }

    coord_t start = { .x = 0, .y = 0, };
    dm_clear_map_visibility(gbl_game->current_map, &start, &gbl_game->current_map->sett.size);
    sgt_calculate_all_light_sources(gbl_game->current_map);
    sgt_calculate_player_sight(gbl_game->current_map, gbl_game->player_data.player);

    update_screen();
}

static unsigned int test_astar_callback(void *vmap, coord_t *coord) {
    unsigned int cost = PF_BLOCKED;
    if (vmap == NULL) return cost;
    if (coord == NULL) return cost;

    struct dm_map *map = (struct dm_map *) vmap;
    struct dm_map_entity *me = dm_get_map_me(coord, map);
    struct tl_tile *te = me->tile;

    if (TILE_HAS_ATTRIBUTE(te,TILE_ATTR_BORDER) == true) return cost;

    if (TILE_HAS_ATTRIBUTE(te,TILE_ATTR_TRAVERSABLE) == true) {
        cost = te->movement_cost;
    }

    if (me->monster != NULL) {
        cost += me->monster->idle_counter;
    }

    if (me->effect != NULL) {
        cost += 100;
    }

    me->test_var = TERM_COLOUR_UMBER;

    return cost;
}

static void cmd_test_a_star(char *input) {
    FIX_UNUSED(input);
    struct msr_monster *player = gbl_game->player_data.player;
    struct dm_map *map = gbl_game->current_map;

    coord_t p_pos = player->pos;
    coord_t e_pos = p_pos;
    int scr_x = get_viewport(p_pos.x, map_win->cols, map->sett.size.x);
    int scr_y = get_viewport(p_pos.y, map_win->lines, map->sett.size.y);

    int ch;
    bool examine_mode = true;
    while( ( (ch = inp_get_input(gbl_game->input)) != INP_KEY_ESCAPE) && (examine_mode == true) ) {
        switch (ch) {
            case INP_KEY_UP_LEFT:    e_pos.y--; e_pos.x--; break;
            case INP_KEY_UP:         e_pos.y--; break;
            case INP_KEY_UP_RIGHT:   e_pos.y--; e_pos.x++; break;
            case INP_KEY_RIGHT:      e_pos.x++; break;
            case INP_KEY_DOWN_RIGHT: e_pos.y++; e_pos.x++; break;
            case INP_KEY_DOWN:       e_pos.y++; break;
            case INP_KEY_DOWN_LEFT:  e_pos.y++; e_pos.x--; break;
            case INP_KEY_LEFT:       e_pos.x--; break;

            case INP_KEY_HELP:
            case INP_KEY_ESCAPE:
            case INP_KEY_QUIT:
            case INP_KEY_NO:
            case INP_KEY_YES:
            case INP_KEY_EXAMINE:    examine_mode = false; break;
            default: break;
        }
        if (examine_mode == false) break;

        if (e_pos.y < 0) e_pos.y = 0;
        if (e_pos.y >= map->sett.size.y) e_pos.y = map->sett.size.y -1;
        if (e_pos.x < 0) e_pos.x = 0;
        if (e_pos.x >= map->sett.size.x) e_pos.x = map->sett.size.x -1;

        chtype oldch = mvwinch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x);
        mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, (oldch & 0xFF) | get_colour(TERM_COLOUR_BG_RED) );
        wrefresh(map_win->win);
        mvwaddch(map_win->win, e_pos.y - scr_y, e_pos.x - scr_x, oldch);
    }

    struct pf_settings pf_set = {
        .map_start = {
            .x = 0,
            .y = 0,
        },
        .map_end = {
            .x = map->sett.size.x,
            .y = map->sett.size.y,
        },
        .map = map,
        .pf_traversable_callback = test_astar_callback,
    };

    struct pf_context *pf_ctx = pf_init(&pf_set);
    if (pf_astar_map(pf_ctx, &p_pos, &e_pos) ) {
        coord_t *path;
        int len = pf_calculate_path(pf_ctx, &p_pos, &e_pos, &path);
        for (int i = 0; i < len; i++) {
            struct dm_map_entity *me = dm_get_map_me(&path[i], map);
            me->test_var = TERM_COLOUR_L_TEAL;
        }
        free(path);
    }
    free(pf_ctx);

    update_screen();
}

static void cmd_list_items(char *input) {
    FIX_UNUSED(input);
    struct itm_item *item = NULL;
    while ( (item = itmlst_get_next_item(item) ) != NULL) {
        if (item->owner_type == ITEM_OWNER_NONE) {
            lg_wizard("%10s %6" PRIu32 " %6" PRIu32 "  (no owner)", item->sd_name, item->uid, item->tid);
        }
        else if (item->owner_type == ITEM_OWNER_MAP) {
            lg_wizard("%10s %6" PRIu32 " %6" PRIu32"  (pos %d,%d)", item->sd_name, item->uid, item->tid, item->owner.owner_map_entity->pos.x, item->owner.owner_map_entity->pos.y);
        }
        else if (item->owner_type == ITEM_OWNER_MONSTER) {
            lg_wizard("%10s %6" PRIu32 " %6" PRIu32 "  (uid %" PRIu32 ")", item->sd_name, item->uid, item->tid, item->owner.owner_monster->uid);
        }
    }
}

static void cmd_list_monsters(char *input) {
    FIX_UNUSED(input);
    struct msr_monster *monster = NULL;
    while ( (monster = msrlst_get_next_monster(monster) ) != NULL) {
        lg_wizard("%10s %6u %6" PRIu32" (pos %d,%d)", monster->sd_name, monster->uid, monster->tid, monster->pos.x, monster->pos.y);
    }
}

static void cmd_list_status_effects(char *input) {
    FIX_UNUSED(input);
    lg_wizard("not implemented");
}

static void cmd_inspect_item(char *input) {
    if (input == NULL) {
        lg_wizard("Usage: inspect_item <uid>");
        return;
    }
    int uid = strtol(input, NULL, 10);
    struct itm_item *item = itmlst_item_by_uid(uid);
    if (item == NULL) {
        lg_wizard("No item with uid %" PRIu32, uid);
        return;
    }

    itm_verify_item(item);
    if (item->owner_type == ITEM_OWNER_NONE) {
        lg_wizard("%10s %6" PRIu32 " %6" PRIu32 "  (no owner)", item->sd_name, item->uid, item->tid);
    }
    else if (item->owner_type == ITEM_OWNER_MAP) {
        lg_wizard("%10s %6" PRIu32 " %6" PRIu32"  (pos %d,%d)", item->sd_name, item->uid, item->tid, item->owner.owner_map_entity->pos.x, item->owner.owner_map_entity->pos.y);
    }
    else if (item->owner_type == ITEM_OWNER_MONSTER) {
        lg_wizard("%10s %6" PRIu32 " %6" PRIu32 "  (uid %" PRIu32 ")", item->sd_name, item->uid, item->tid, item->owner.owner_monster->uid);
    }
    lg_wizard("spawn weight: %d, level: %d", item->spawn_weight, item->spawn_level);
    lg_wizard("quality: %d", item->quality);
    lg_wizard("weight: %d", item->weight);
    lg_wizard("cost: %d", item->cost);
    lg_wizard("long name: %s", item->ld_name);
    lg_wizard("description: %s", item->description);
    lg_wizard("icon: '%c'", item->icon);
    lg_wizard("use delay: %d", item->use_delay);
    lg_wizard("stack quantity: %d/%d", item->stacked_quantity, item->max_quantity);
    lg_wizard("identified : %d (last try: %d)", item->identified, item->identify_last_try);
    lg_wizard("age : %d", item->age);
    lg_wizard("energy(action %d, permanent: %d) : %d", item->energy_action, item->permanent_energy, item->energy);
    lg_wizard("dropable : %d", item->dropable);

    switch (item->item_type) {
        case ITEM_TYPE_WEAPON: {
            struct item_weapon_specific *wpn = &item->specific.weapon;
            char mod = '+';
            int add = wpn->dmg_addition;
            if (add < 0) mod = '-';
            lg_wizard("%c%d\n", mod, abs(add) );
            if (wpn->nr_dmg_die == 0) lg_wizard("1D5");
            else lg_wizard("damage %dD10 %c %d", wpn->nr_dmg_die, mod, abs(add) );

            if (wpn_is_type(item, WEAPON_TYPE_RANGED) || wpn_is_type(item, WEAPON_TYPE_THROWN) ) {
                lg_wizard("range: %d\n", wpn->range);
            }

            if (wpn_is_type(item, WEAPON_TYPE_RANGED) ) {
                lg_wizard("- Magazine size %d\n", wpn->magazine_sz);
                if (wpn_uses_ammo(item) ) lg_wizard("- Uses %s\n", wpn_ammo_string(wpn->ammo_type) );

                int single = wpn->rof[WEAPON_ROF_SETTING_SINGLE];
                int semi = wpn->rof[WEAPON_ROF_SETTING_SEMI];
                int aut = wpn->rof[WEAPON_ROF_SETTING_AUTO];

                if (semi > 0 || aut > 0) {
                    char semi_str[4]; snprintf(semi_str, 3, "%d", semi);
                    char auto_str[4]; snprintf(auto_str, 3, "%d", aut);
                    lg_wizard("- Rate of Fire (%s/%s/%s)\n",
                            (single > 0) ? "S" : "-", (semi > 0) ? semi_str : "-", (aut > 0) ? auto_str : "-");
                }

                lg_wizard("\n");
                if (wpn->magazine_left == 0) {
                    lg_wizard("The weapon is empty.\n");
                }
                else {
                    struct itm_item *ammo = itm_create(wpn->ammo_used_tid);
                    lg_wizard("It is currently loaded with %s.\n\n", ammo->ld_name);
                    itm_destroy(ammo);
                }

                if (wpn->jammed == true) {
                    lg_wizard("It is jammed.\n");
                }
            }

            if (wpn->wpn_talent != TLT_NONE) {
                lg_wizard("This weapon requires the %s talent.\n", msr_talent_names(wpn->wpn_talent) );
            }

            if (wpn->special_quality != 0) {
                lg_wizard("\n");
                lg_wizard("Weapon qualities:\n");

                if (wpn->penetration > 0) {
                    lg_wizard("- Armour Penetration (%d).\n", wpn->penetration);
                }

                for (int i = 0; i < WPN_SPCQLTY_MAX; i++) {
                    if (wpn_has_spc_quality(item, i) )  {
                        lg_wizard("- %s.\n", wpn_spec_quality_name(i) );
                    }
                }
            }
        } break;
        case ITEM_TYPE_WEARABLE: {
            struct item_wearable_specific *wrbl = &item->specific.wearable;
            lg_wizard("armour: %d\n", wrbl->damage_reduction);

            lg_wizard("locations: ");
            for (enum inv_locations i = 1; i < INV_LOC_MAX; i <<= 1) {
                if ( (wrbl->locations & i) > 0) {
                    lg_wizard("- %s", inv_location_name(wrbl->locations & i) );
                }
            }

            if (wrbl->special_quality != 0) {
                lg_wizard("qualities:\n");

                for (int i = 0; i < WBL_SPCQLTY_MAX; i++) {
                    if (wbl_has_spc_quality(item, i) )  {
                        lg_wizard("- %s.\n", wbl_spec_quality_name(i) );
                    }
                }
            }
        } break;
        case ITEM_TYPE_TOOL: {
            struct item_tool_specific *tool = &item->specific.tool;
            if (tool->energy > 0) {
                int energy_pc = (item->energy * 100) / tool->energy;
                lg_wizard("energy left %d\%\n", energy_pc);
            }

        }break;
        case ITEM_TYPE_AMMO: {
            struct item_ammo_specific *ammo = &item->specific.ammo;
            if (ammo->energy > 0) {
                int energy_pc = (ammo->energy_left * 100) / ammo->energy;
                lg_wizard("energy left %d\%\n", energy_pc);
            }
            lg_wizard("provides %s\n", wpn_ammo_string(ammo->ammo_type) );
        } break;
        case ITEM_TYPE_FOOD: break;
        default: break;
    }
}

static void cmd_inspect_monster(char *input) {
    if (input == NULL) {
        lg_wizard("Usage: inspect_monster <uid>");
        return;
    }

    int uid = strtol(input, NULL, 10);
    struct msr_monster *monster = msr_get_monster_by_uid(uid);
    if (monster == NULL) {
        lg_wizard("No monster with uid %" PRIu32, uid);
        return;
    }

    msr_verify_monster(monster);
    lg_wizard("%10s %6u %6" PRIu32" (pos %d,%d)", monster->sd_name, monster->uid, monster->tid, monster->pos.x, monster->pos.y);
    lg_wizard("long name: %s", monster->ld_name);
    lg_wizard("description: %s", monster->description);
    lg_wizard("icon: '%c'", monster->icon);
    lg_wizard("fate points: %d", monster->fate_points);
    lg_wizard("insanity points: %d", monster->insanity_points);
    lg_wizard("corruption points: %d", monster->corruption_points);
    lg_wizard("energy: %d", monster->energy);
    lg_wizard("idle_counter: %d", monster->idle_counter);
    lg_wizard("traits[%d]: %" PRIu64, monster->creature_traits);
    for (int i = MSR_TALENT_TIER_T1; i < MSR_TALENT_TIER_MAX; i++) {
        lg_wizard("talents[%d]: %" PRIu64, monster->talents[i]);
        for (int x = 0; x < MSR_TALENTS_PER_TIER; x++) {
            enum msr_talents t = MSR_TALENT(i, x);
            if (t == TLT_NONE) continue;

            if (msr_has_talent(monster, t) ) {
                lg_wizard("talent %s", msr_talent_names(t) );
            }
        }
    }
    for (unsigned int i = 0; i < MSR_SKILLS_MAX; i++) {
        if (msr_has_skill(monster, i) ) {
            enum msr_skill_rate skillrate = msr_has_skill(monster,  i);
            lg_wizard("%-20s %s", msr_skill_names(i),  msr_skillrate_names(skillrate));
        }
    }
}

static void cmd_inspect_se(char *input) {
    FIX_UNUSED(input);
    lg_wizard("not implemented");
}
