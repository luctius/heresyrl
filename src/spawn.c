#include <assert.h>
#include <sys/param.h>

#include "spawn.h"
#include "logging.h"
#include "game.h"
#include "dungeon_creator.h"
#include "tiles.h"
#include "items.h"
#include "monster.h"
#include "spawn_static.h"
#include "random.h"
#include "ai.h"
#include "dowear.h"

#include "spawn_static.c"

static int spawn_item(double roll) {
    int sz = ARRAY_SZ(item_weights);
    double prob_arr[sz];
    double cumm_prob_arr[sz];
    double sum = 0;

    for (int i = 0; i < sz; i++) {
        sum += item_weights[i].weight;
    }
    double cumm = 0;
    for (int i = 0; i < sz; i++) {
        prob_arr[i] = item_weights[i].weight / sum;
        cumm += prob_arr[i];
        cumm_prob_arr[i] = cumm;
    }
    int idx = -1;
    for (int i = sz; i > 0; i--) {
        if (roll < cumm_prob_arr[i-1]) idx = i-1;
        else return idx;
    }
    return idx;
}

static int spawn_monster(double roll) {
    int sz = ARRAY_SZ(monster_weights);
    double prob_arr[sz];
    double cumm_prob_arr[sz];
    double sum = 0;

    for (int i = 0; i < sz; i++) {
        sum += monster_weights[i].weight;
    }
    double cumm = 0;
    for (int i = 0; i < sz; i++) {
        prob_arr[i] = monster_weights[i].weight / sum;
        cumm += prob_arr[i];
        cumm_prob_arr[i] = cumm;
    }
    int idx = -1;
    for (int i = sz; i > 0; i--) {
        if (roll < cumm_prob_arr[i-1]) idx = i-1;
        else return idx;
    }
    return idx;
}

bool spwn_add_item_to_monster(struct msr_monster *monster, struct spwn_monster_item *sitem, struct random *r) {
    int nr = 1;
    if (sitem->min != sitem->max) {
        nr = (random_int32(r) % (sitem->max - sitem->min) ) + sitem->min;
    }

    if (nr >= 1) {
        struct itm_item *item = itm_create(sitem->id);
        if (item != NULL) {
            item->stacked_quantity = MIN(nr, item->max_quantity);
            assert(msr_give_item(monster, item) == true);
            if (sitem->wear == true) {
                assert(dw_wear_item(monster, item) == true);
                return true;
            }
        }
    }
    return false;
}

bool spwn_populate_map(struct dc_map *map, struct random *r, uint32_t monster_chance, uint32_t item_chance) {
    if (dc_verify_map(map) == false) return false;
    if (r == NULL) return false;
    coord_t c;
    int idx;

    struct msr_monster *player = gbl_game->player_data.player;
    int nogo_radius = msr_get_medium_sight_range(player);

    for (int xi = 0; xi < map->size.x; xi++) {
        for (int yi = 0; yi < map->size.y; yi++) {
            c = cd_create(xi,yi);
            if (cd_pyth(&player->pos, &c) <= nogo_radius) continue; /* no npc's too close to the player*/

            if ( (random_int32(r) % 10000) <= monster_chance) {
                if (TILE_HAS_ATTRIBUTE(sd_get_map_me(&c,map)->tile, TILE_ATTR_TRAVERSABLE) == true) {
                    idx = spawn_monster(random_float(r) );
                    struct msr_monster *monster = msr_create(monster_weights[idx].id);

                    msr_insert_monster(monster, map, &c);
                    ai_monster_init(monster);

                    struct spwn_monster_item *items = monster_weights[idx].items;
                    int i = 0;
                    while (items[i].max != 0) {
                        spwn_add_item_to_monster(monster, &items[i], r);
                        i++;
                    }
                }
            }

            if ( (random_int32(r) % 10000) <= item_chance) {
                if (TILE_HAS_ATTRIBUTE(sd_get_map_me(&c,map)->tile, TILE_ATTR_TRAVERSABLE) == true) {
                    idx = spawn_item(random_float(r) );
                    struct itm_item *item = itm_create(item_weights[idx].id);

                    itm_insert_item(item, map, &c);
                }
            }
        }
    }

    return true;
}
