#include <ncurses.h>

#include "game.h"
#include "items.h"
#include "monster.h"
#include "random.h"
#include "sight.h"

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

            game->sight = sgt_init();

            msr_monster_list_init();
            itm_items_list_init();

            game->current_map = dc_alloc_map(x,y);
            dc_generate_map(game->current_map, DC_DUNGEON_TYPE_CAVE, 1, random_genrand_int32(game->map_random) );

            game->player_data.age = 0;
            game->player_data.player = msr_create();
            game->player_data.player->icon = '@';
            game->player_data.player->icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL) | A_BOLD;

            int xpos, ypos;
            if (dc_tile_instance(game->current_map, TILE_TYPE_STAIRS_UP, 0, &xpos, &ypos) == false) exit(1);
            if (msr_insert_monster(game->player_data.player, game->current_map, xpos, ypos) == false) exit(1);
        }
    }
}

void game_new_turn(void) {
    game->player_data.age++;

    int sight_range = msr_get_near_sight_range(game->player_data.player) + msr_get_far_sight_range(game->player_data.player);
    int xs = game->player_data.player->x_pos - sight_range;
    int ys = game->player_data.player->y_pos - sight_range;
    int xe = game->player_data.player->x_pos + sight_range;
    int ye = game->player_data.player->y_pos + sight_range;
    xs = (xs <= 0) ? xs : 0;
    ys = (ys >= 0) ? ys : 0;
    xe = (xe >= game->current_map->x_sz -1) ? xe : game->current_map->x_sz -1;
    ye = (ye >= game->current_map->y_sz -1) ? ye : game->current_map->y_sz -1;

    //dc_clear_map_visibility(game->current_map, xs, ys, xe, ye);
    dc_clear_map_visibility(game->current_map, 0,0, game->current_map->x_sz, game->current_map->y_sz);
    sgt_calculate_all_light_sources(game->sight, game->current_map);
    sgt_calculate_player_sight(game->sight, game->current_map, game->player_data.player);
}

void game_exit() {
    msr_die(game->player_data.player, game->current_map);
    dc_free_map(game->current_map);

    msr_monster_list_exit();
    itm_items_list_exit();

    sgt_exit(game->sight);

    random_exit(game->game_random);
    random_exit(game->ai_random);
    random_exit(game->item_random);
    random_exit(game->monster_random);
    random_exit(game->map_random);

    free(game);
}

