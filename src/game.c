#include "game.h"
#include "cmdline.h"
#include "dungeon_creator.h"
#include "items.h"
#include "monster.h"
#include "random.h"
#include "sight.h"
#include "player.h"
#include "tiles.h"
#include "save.h"
#include "load.h"
#include "spawn.h"
#include "monster_action.h"

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
            ma_init();

            gbl_game->running = true;

            gbl_game->sight = sgt_init();
        }
    }
}

bool game_load(void) {
    bool loaded = false;
    if (gbl_game == NULL) return false;

    if (gbl_game->game_random != NULL) {
        random_exit(gbl_game->game_random);
        random_exit(gbl_game->spawn_random);
        random_exit(gbl_game->map_random);
        random_exit(gbl_game->ai_random);
        gbl_game->game_random = NULL;
        gbl_game->spawn_random = NULL;
        gbl_game->map_random = NULL;
        gbl_game->ai_random = NULL;
    }

    if (gbl_game->args_info->no_load_flag == false) {
        loaded = ld_read_save_file(gbl_game->args_info->save_file_arg, gbl_game);
        if (loaded == true) {
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "game", "Game loaded from %s.", gbl_game->args_info->save_file_arg);
        }
        else {
            lg_printf_l(LG_DEBUG_LEVEL_WARNING, "game", "Failed to load game from %s.", gbl_game->args_info->save_file_arg);
        }
    }

    return loaded;
}

bool game_init_map(void) {
    if (gbl_game == NULL) return false;

    if (gbl_game->game_random == NULL) {
        gbl_game->game_random = random_init_genrand(gbl_game->initial_seed);
        gbl_game->spawn_random = random_init_genrand(random_int32(gbl_game->game_random));
        gbl_game->map_random = random_init_genrand(random_int32(gbl_game->game_random));
        gbl_game->ai_random = random_init_genrand(random_int32(gbl_game->game_random));
    }

    int x = 100;
    int y = 100;

    if (gbl_game->current_map == NULL) {
        gbl_game->current_map = dc_alloc_map(x,y);
        dc_generate_map(gbl_game->current_map, DC_DUNGEON_TYPE_CAVE, 1, random_int32(gbl_game->map_random) );
    }

    plr_init(&gbl_game->player_data, "Tester", MSR_RACE_HUMAN, MSR_GENDER_MALE);
    gbl_game->player_data.player->is_player = true;

    coord_t c = cd_create(0,0);
    if (cd_equal(&gbl_game->player_data.player->pos, &c) == true) {
        if (dc_tile_instance(gbl_game->current_map, TILE_TYPE_STAIRS_UP, 0, &c) == false) exit(1);
        if (msr_insert_monster(gbl_game->player_data.player, gbl_game->current_map, &c) == false) exit(1);
        spwn_populate_map(gbl_game->current_map, gbl_game->spawn_random, 50, 10);
    }

    dc_clear_map_visibility(gbl_game->current_map, &c, &gbl_game->current_map->size);
    sgt_calculate_all_light_sources(gbl_game->sight, gbl_game->current_map);
    sgt_calculate_player_sight(gbl_game->sight, gbl_game->current_map, gbl_game->player_data.player);

    return true;
}

bool game_new_tick(void) {
    if (gbl_game == NULL) return false;
    gbl_game->turn += MSR_ENERGY_TICK;

    coord_t zero = cd_create(0,0);
    dc_clear_map_visibility(gbl_game->current_map, &zero, &gbl_game->current_map->size);
    sgt_calculate_all_light_sources(gbl_game->sight, gbl_game->current_map);
    sgt_calculate_player_sight(gbl_game->sight, gbl_game->current_map, gbl_game->player_data.player);
    return true;
}

bool game_exit() {
    if (gbl_game == NULL) return false;

    if (gbl_game->args_info->no_save_flag == false) {
        if (sv_save_game(gbl_game->args_info->save_file_arg, gbl_game) == true) {
            lg_printf("Game Saved.");
        }
    }

    ma_exit();
    dc_free_map(gbl_game->current_map);

    msrlst_monster_list_exit();
    itmlst_items_list_exit();

    sgt_exit(gbl_game->sight);

    random_exit(gbl_game->game_random);
    random_exit(gbl_game->ai_random);
    random_exit(gbl_game->spawn_random);
    random_exit(gbl_game->map_random);

    free(gbl_game);
    return true;
}

