#include "game.h"
#include "items.h"
#include "monster.h"
#include "random.h"

struct gm_game *game = NULL;

void game_init(unsigned long initial_seed) {
    if (game == NULL) {
        game = malloc(sizeof(game));
        if (game != NULL) {
            int x = 80;
            int y = 50;
            game->game_random = random_init_genrand(initial_seed);
            game->item_random = random_init_genrand(random_genrand_int32(game->game_random));
            game->monster_random = random_init_genrand(random_genrand_int32(game->game_random));
            game->map_random = random_init_genrand(random_genrand_int32(game->game_random));
            game->ai_random = random_init_genrand(random_genrand_int32(game->game_random));


            msr_monster_list_init();
            itm_items_list_init();

            game->current_map = dc_alloc_map(x,y);
            dc_generate_map(game->current_map, DC_DUNGEON_TYPE_CAVE, 1, random_genrand_int32(game->map_random) );

            game->player_data.player = msr_create();
            game->player_data.player->icon = '@';
            game->player_data.player->colour = DPL_COLOUR_NORMAL;

            int xpos, ypos;
            if (dc_tile_instance(game->current_map, TILE_TYPE_STAIRS_UP, 0, &xpos, &ypos) == false) exit(1);
            if (msr_insert_monster(game->player_data.player, game->current_map, xpos, ypos) == false) exit(1);
        }
    }
}

void game_exit() {
    msr_die(game->player_data.player, game->current_map);
    dc_free_map(game->current_map);

    msr_monster_list_exit();
    itm_items_list_exit();

    free(game->game_random);
    free(game->ai_random);
    free(game->item_random);
    free(game->monster_random);
    free(game->map_random);

    free(game);
}

