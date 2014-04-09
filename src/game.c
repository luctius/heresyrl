#include "game.h"
#include "cmdline.h"
#include "options.h"

#include "input.h"
#include "player.h"
#include "tiles.h"
#include "save.h"
#include "load.h"
#include "spawn.h"
#include "random.h"
#include "dungeon/dungeon_map.h"
#include "items/items.h"
#include "monster/monster.h"
#include "monster/monster_action.h"
#include "fov/sight.h"

#define LOAD 1
#define SAVE 1

struct gm_game *gbl_game = NULL;

void game_init(struct pl_player *plr, unsigned long initial_seed) {
    if (gbl_game == NULL) {
        gbl_game = calloc(1, sizeof(struct gm_game));
        if (gbl_game != NULL) {
            gbl_game->initial_seed = initial_seed;
            gbl_game->turn = 0;
            msrlst_monster_list_init();
            itmlst_items_list_init();
            tt_init();

            gbl_game->sight = sgt_init();
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
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "game", "Game loaded from %s.", options.save_file_name);
        }
        else {
            lg_printf_l(LG_DEBUG_LEVEL_WARNING, "game", "Failed to load game from %s.", options.save_file_name);
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

    if (gbl_game->current_map == NULL) {
        gbl_game->current_map = dm_alloc_map(x,y);
        //dm_generate_map(gbl_game->current_map, DM_DUNGEON_TYPE_SIMPLE, 1, random_int32(gbl_game->random) );
        dm_generate_map(gbl_game->current_map, DM_DUNGEON_TYPE_CAVE, 1, random_int32(gbl_game->random) );
    }

    //plr_init(&gbl_game->player_data, "Tester", MSR_RACE_HUMAN, MSR_GENDER_MALE);
    plr_init(&gbl_game->player_data);

    coord_t c = cd_create(0,0);
    if (cd_equal(&gbl_game->player_data.player->pos, &c) == true) {
        if (dm_tile_instance(gbl_game->current_map, TILE_TYPE_STAIRS_UP, 0, &c) == false) exit(1);
        if (msr_insert_monster(gbl_game->player_data.player, gbl_game->current_map, &c) == false) exit(1);
        spwn_populate_map(gbl_game->current_map, gbl_game->random, 100, 10);
    }

    dm_clear_map_visibility(gbl_game->current_map, &c, &gbl_game->current_map->size);
    sgt_calculate_all_light_sources(gbl_game->sight, gbl_game->current_map);
    sgt_calculate_player_sight(gbl_game->sight, gbl_game->current_map, gbl_game->player_data.player);
    gbl_game->running = true;

    if (gbl_game->player_data.xp_current > TT_ENERGY_TURN) GM_msg("You have XP to spend, press '@' to level-up.");

    return gbl_game->running;
}

bool game_new_tick(void) {
    if (gbl_game == NULL) return false;
    gbl_game->turn += TT_ENERGY_TICK;

    if (options.play_recording) {
        if (gbl_game->turn > (options.play_stop * TT_ENERGY_TURN) ) {
            options.play_recording = false;
            options.refresh = true;
            inp_keylog_stop(gbl_game->input);

            update_screen();
        }
    }

    return true;
}

bool game_exit() {
    if (gbl_game == NULL) return false;

    if (options.debug_no_save == false) {
        if (sv_save_game(options.save_file_name, gbl_game) == true) {
            lg_print("Game Saved.");
        }
    }

    if (gbl_game->current_map != NULL) dm_free_map(gbl_game->current_map);

    tt_exit();

    msrlst_monster_list_exit();
    itmlst_items_list_exit();

    inp_exit(gbl_game->input);
    sgt_exit(gbl_game->sight);

    random_exit(gbl_game->random);

    free(gbl_game);
    return true;
}

