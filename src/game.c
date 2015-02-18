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

            msrlst_monster_list_init();
            itmlst_items_list_init();
            tt_init();
            se_init();

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
        loaded = ld_read_save_file(options.save_file_name, gbl_game);
        if (loaded == true) {
            lg_debug("Game loaded from %s.", options.save_file_name);
        }
        else {
            lg_warning("Failed to load game from %s.", options.save_file_name);
        }
    }

    if (gbl_game->random == NULL) {
        gbl_game->random = random_init_genrand(gbl_game->initial_seed);
    }

    return loaded;
}

bool game_init_map(void) {
    if (gbl_game == NULL) return false;

    int x = 100;
    int y = 100;
    bool new_map = false;

    if (plr_init(&gbl_game->player_data) == false) {
        return false;
    }

    if (gbl_game->current_map == NULL) {
        new_map = true;
        gbl_game->current_map = dm_alloc_map(x,y);

        int rand = random_d100(gbl_game->random);
        if (rand < 60) {
            dm_generate_map(gbl_game->current_map, DUNGEON_TYPE_CAVE, 1, random_int32(gbl_game->random), true);
        }
        else if (rand < 80) {
            dm_generate_map(gbl_game->current_map, DUNGEON_TYPE_TUNNEL, 1, random_int32(gbl_game->random), true );
        }
        else {
            dm_generate_map(gbl_game->current_map, DUNGEON_TYPE_PLAIN, 1, random_int32(gbl_game->random), true );
        }

        System_msg("Press '?' to view the help screen.");
    }

    coord_t c = cd_create(0,0);
    if (cd_equal(&gbl_game->player_data.player->pos, &c) == true) {
        if (dm_tile_instance(gbl_game->current_map, TILE_TYPE_STAIRS_UP, 0, &c) == false) exit(1);
        if (msr_insert_monster(gbl_game->player_data.player, gbl_game->current_map, &c) == false) exit(1);
        if (new_map) cr_generate_allies(gbl_game->player_data.career, gbl_game->player_data.player, gbl_game->current_map);
    }

    dm_clear_map_visibility(gbl_game->current_map, &c, &gbl_game->current_map->size);
    sgt_calculate_all_light_sources(gbl_game->current_map);
    sgt_calculate_player_sight(gbl_game->current_map, gbl_game->player_data.player);
    gbl_game->running = true;

    return gbl_game->running;
}

bool game_new_tick(void) {
    if (gbl_game == NULL) return false;
    gbl_game->turn += TT_ENERGY_TICK;

    if (options.play_recording) {
        if (options.play_stop > 0) {
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

    /* delete save game */
    if (options.debug_no_save == false) {
        FILE *f = fopen(options.save_file_name, "w");
        fclose(f);
    }

    struct pl_player *plr = &gbl_game->player_data;
    if (plr != NULL) {
        if (plr->player != NULL) {
            if (options.debug_no_save == false) {
                if (plr->player->dead == false) {
                    if (sv_save_game(options.save_file_name, gbl_game) == true) {
                        lg_print("Game Saved.");
                    }
                }
            }

            free(plr->player->unique_name);
        }
    }


    if (gbl_game->current_map != NULL) dm_free_map(gbl_game->current_map);

    /* order is important due to freeing of items*/
    msrlst_monster_list_exit();
    itmlst_items_list_exit();
    se_exit();
    tt_exit();

    inp_exit(gbl_game->input);

    random_exit(gbl_game->random);

    free(gbl_game);
    return true;
}

