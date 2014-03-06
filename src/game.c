#include "game.h"
#include "dungeon_creator.h"
#include "items.h"
#include "monster.h"
#include "random.h"
#include "sight.h"
#include "player.h"
#include "tiles.h"
#include "save.h"

struct gm_game *game = NULL;

void game_init(struct pl_player *plr, unsigned long initial_seed) {
    if (game == NULL) {
        game = calloc(1, sizeof(struct gm_game));
        if (game != NULL) {
            game->game_random = random_init_genrand(initial_seed);
            game->item_random = random_init_genrand(random_genrand_int32(game->game_random));
            game->monster_random = random_init_genrand(random_genrand_int32(game->game_random));
            game->map_random = random_init_genrand(random_genrand_int32(game->game_random));
            game->ai_random = random_init_genrand(random_genrand_int32(game->game_random));

            game->sight = sgt_init();

            msrlst_monster_list_init();
            itmlst_items_list_init();
        }
    }
}

bool game_init_map(void) {
    if (game == NULL) return false;
    int x = 100;
    int y = 100;

    game->current_map = dc_alloc_map(x,y);
    dc_generate_map(game->current_map, DC_DUNGEON_TYPE_CAVE, 1, random_genrand_int32(game->map_random) );

    plr_init(&game->player_data, "Tester", MSR_RACE_HUMAN, MSR_GENDER_MALE);
    game->player_data.player->is_player = true;

    coord_t c;
    if (dc_tile_instance(game->current_map, TILE_TYPE_STAIRS_UP, 0, &c) == false) exit(1);
    if (msr_insert_monster(game->player_data.player, game->current_map, &c) == false) exit(1);
    return true;
}

bool game_new_turn(void) {
    if (game == NULL) return false;
    game->player_data.age++;

    coord_t zero = cd_create(0,0);
    dc_clear_map_visibility(game->current_map, &zero, &game->current_map->size);
    sgt_calculate_all_light_sources(game->sight, game->current_map);
    sgt_calculate_player_sight(game->sight, game->current_map, game->player_data.player);
    return true;
}

bool game_exit() {
    if (game == NULL) return false;
    sv_save_game("/tmp/heresyrl.save", game);

    msr_die(game->player_data.player, game->current_map);
    dc_free_map(game->current_map);

    msrlst_monster_list_exit();
    itmlst_items_list_exit();

    sgt_exit(game->sight);

    random_exit(game->game_random);
    random_exit(game->ai_random);
    random_exit(game->item_random);
    random_exit(game->monster_random);
    random_exit(game->map_random);

    free(game);
    return true;
}

