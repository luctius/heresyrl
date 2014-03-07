#include "spawn.h"
#include "logging.h"
#include "dungeon_creator.h"
#include "tiles.h"
#include "items.h"
#include "monster.h"
#include "items_static.h"
#include "monster_static.h"
#include "random.h"

struct spwn_item {
    enum item_ids id;
    int weight;
};

struct spwn_monster {
    enum msr_ids id;
    int weight;
};

static struct spwn_item item_weights[] = {
    {.weight=1, .id=ITEM_ID_AVERAGE_TORCH, },
    {.weight=10, .id=ITEM_ID_AVERAGE_STUB_AUTOMATIC, },
    {.weight=15, .id=ITEM_ID_AVERAGE_STUB_REVOLVER, },
};

static struct spwn_monster monster_weights[] = {
    {.weight=1, .id=MSR_ID_BASIC_FERAL, },
};

uint32_t spawn_item(double roll) {
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
    int idx = 0;
    for (int i = sz; i > 0; i--) {
        if (roll < cumm_prob_arr[i-1]) idx = item_weights[i-1].id;
        else return idx;
    }
    return 0;
}

uint32_t spawn_monster(double roll) {
    int sz = ARRAY_SZ(monster_weights);
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
    int idx = 0;
    for (int i = sz; i > 0; i--) {
        if (roll < cumm_prob_arr[i]) idx = monster_weights[i-1].id;
        else return idx;
    }
    return 0;
}

bool spwn_populate_map(struct dc_map *map, struct random *r, int generations) {
    if (map == NULL) return false;
    if (r == NULL) return false;

    for (int i = 0; i < generations; i++) {
        if ( (random_int32(r) % 10000) <= 20) {
            int idx = spawn_monster(random_float(r) );
            coord_t c;
            int try = 10;
            struct msr_monster *monster = msr_create(idx);

            for (int j = 0; j < try; j++) {
                c = cd_create(random_int32(r) % map->size.x,  random_int32(r) % map->size.y);
                if (TILE_HAS_ATTRIBUTE(sd_get_map_me(&c,map)->tile, TILE_ATTR_TRAVERSABLE) == true) {
                    msr_insert_monster(monster, map, &c);
                    j = try;
                }
            }
        }
    }

    for (int i = 0; i < generations; i++) {
        if ( (random_int32(r) % 10000) <= 20) {
            int idx = spawn_item(random_float(r) );
            bool spawned = false;
            coord_t c;
            int try = 10;
            struct itm_item *item = itm_create(idx);

            for (int j = 0; j < try; j++) {
                c = cd_create(random_int32(r) % map->size.x,  random_int32(r) % map->size.y);
                if (TILE_HAS_ATTRIBUTE(sd_get_map_me(&c,map)->tile, TILE_ATTR_TRAVERSABLE) == true) {
                    itm_insert_item(item, map, &c);
                    j = try;
                }
            }
        }
    }

    return true;
}
