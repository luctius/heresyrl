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

#include "game.h"
#include "cmdline.h"
#include "options.h"

#include "input.h"
#include "player.h"
#include "save.h"
#include "load.h"
#include "random.h"
#include "dungeon/tiles.h"
#include "dungeon/dungeon_map.h"
#include "dungeon/spawn.h"
#include "items/items.h"
#include "monster/monster.h"
#include "monster/monster_action.h"
#include "fov/sight.h"
#include "ui/ui.h"
#include "quests/quests.h"
#include "careers/careers.h"

#define LOAD 1
#define SAVE 1

struct gm_game *gbl_game = NULL;

void game_init(struct pl_player *plr, unsigned long initial_seed) {
    FIX_UNUSED(plr);

    if (gbl_game == NULL) {
        gbl_game = calloc(1, sizeof(struct gm_game));
        if (gbl_game != NULL) {
            gbl_game->initial_seed = initial_seed;
            gbl_game->turn = 0;
            gbl_game->plr_last_turn = 0;

            msrlst_monster_list_init();
            itmlst_items_list_init();
            tt_init();
            se_init();
            ge_init();
            cr_init();

            gbl_game->input = inp_init();
        }
    }
}

bool game_load(void) {
    bool loaded = false;
    if (gbl_game == NULL) return false;

    if (gbl_game->random != NULL) {
        random_exit(gbl_game->random);
        gbl_game->random = NULL;
    }

    if (options.debug_no_load == false) {
        loaded = ld_read_save_file(options.load_file_name, gbl_game);
        if (loaded == true) {
            lg_debug("Game loaded from %s.", options.load_file_name);
        }
        else {
            lg_warning("Failed to load game from %s.", options.load_file_name);
        }
    }

    if (gbl_game->random == NULL) {
        gbl_game->random = random_init_genrand(gbl_game->initial_seed);
    }

    return loaded;
}

bool game_init_map(void) {
    if (gbl_game == NULL) return false;

    bool new_map = false;

    if (plr_init(&gbl_game->player_data) == false) {
        return false;
    }

    if (gbl_game->player_data.quest == NULL) {
        gbl_game->player_data.quest = qst_spawn(1, random_int32(gbl_game->random) );
    }

    if (gbl_game->current_map == NULL) {
        new_map = true;
        struct qst_dungeon *dun = qst_select_dungeon(gbl_game->player_data.quest, random_int32(gbl_game->random));

        struct quest *q = gbl_game->player_data.quest;
        struct dm_spawn_settings spwn_sett = {
            .size = cd_create(dun->size.x, dun->size.y),
            .threat_lvl_min  = q->min_level,
            .threat_lvl_max = q->max_level,
            .item_chance = dun->item_chance,
            .monster_chance = dun->monster_chance,
            .seed = random_int32(gbl_game->random),
            .type = dun->type,
        };

        gbl_game->current_map = dm_generate_map(&spwn_sett);
        dm_populate_map(gbl_game->current_map);
        qst_process_quest_start(gbl_game->player_data.quest, gbl_game->current_map, gbl_game->random);
    }

    coord_t c = cd_create(0,0);
    if (cd_equal(&gbl_game->player_data.player->pos, &c) == true) {
        if (dm_tile_instance(gbl_game->current_map, TILE_TYPE_STAIRS_UP, 0, &c) == false) exit(1);
        if (msr_insert_monster(gbl_game->player_data.player, gbl_game->current_map, &c) == false) exit(1);
    }

    dm_clear_map_visibility(gbl_game->current_map, &c, &gbl_game->current_map->sett.size);
    sgt_calculate_all_light_sources(gbl_game->current_map);
    sgt_calculate_player_sight(gbl_game->current_map, gbl_game->player_data.player);
    gbl_game->running = true;

    return gbl_game->running;
}

void game_save(void) {
    if (options.debug_no_save == false) {
        if (options.debug == false) {
            /* delete save game */
            FILE *f = fopen(options.save_file_name, "w");
            fclose(f);
        }

        struct pl_player *plr = &gbl_game->player_data;
        if (plr != NULL) {
            if (plr->player != NULL) {
                if (plr->player->dead == false) {
                    if (sv_save_game(options.save_file_name, gbl_game) == true) {
                    }
                }
            }
        }
    }
}

bool game_new_tick(void) {
    if (gbl_game == NULL) return false;

    if (options.debug_no_save == false) {
        FILE *f = fopen(options.save_file_name, "w");
        fclose(f);
    }

    gbl_game->turn += TT_ENERGY_TICK;

    if (options.play_recording) {
        if (options.play_stop > -1) {
            if (gbl_game->turn > (options.play_stop * TT_ENERGY_TURN) ) {
                options.play_recording = false;
                options.refresh = true;
                inp_keylog_stop(gbl_game->input);

                update_screen();
            }
        }
    }

    return true;
}

bool game_exit() {
    if (gbl_game == NULL) return false;

    game_save();

    struct pl_player *plr = &gbl_game->player_data;
    if (plr != NULL && plr->player != NULL) {
        if (plr->player->dead) {
            cr_print_morgue_file(plr);
        }
        free(plr->player->unique_name);
    }

    if (gbl_game->current_map != NULL) dm_free_map(gbl_game->current_map);

    /* order is important due to freeing of items*/
    msrlst_monster_list_exit();
    itmlst_items_list_exit();
    ge_exit();
    se_exit();
    tt_exit();
    cr_exit(plr);

    inp_exit(gbl_game->input);

    random_exit(gbl_game->random);

    free(gbl_game);
    return true;
}

void game_cleanup(void) {
    struct msr_monster *player = gbl_game->player_data.player;
    struct inv_inventory *inv =  player->inventory;

    gbl_game->player_data.quest = NULL;

    struct msr_monster *m = NULL;
    while ( (m = msrlst_get_next_monster(m) ) != NULL ) {
        if (m->is_player) continue;
        msr_destroy(m, gbl_game->current_map);
        m = NULL;
    }

    struct ground_effect *g = NULL;
    while ( (g = gelst_get_next(g) ) != NULL ) {
        ge_destroy(g->me);
        g = NULL;
    }

    se_remove_all_non_permanent(player);

    dm_free_map(gbl_game->current_map);
    gbl_game->current_map = NULL;
    player->pos = cd_create(0,0);
}

