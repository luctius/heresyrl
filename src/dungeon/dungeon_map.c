#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <ncurses.h>

#include "heresyrl_def.h"
#include "dungeon_map.h"

#include "tiles.h"
#include "random.h"
#include "ai/ai_utils.h"
#include "ai/pathfinding.h"
#include "inventory.h"
#include "items/items.h"
#include "items/items_static.h"
#include "fov/sight.h"
#include "options.h"

#include "dungeon_cave.h"
#include "dungeon_room.h"
#include "dungeon_plain.h"

extern inline struct dm_map_entity *dm_get_map_me(coord_t *c, struct dm_map *map);
extern inline struct tl_tile *dm_get_map_tile(coord_t *c, struct dm_map *map);
static bool dm_clear_map_unsafe(struct dm_map *map);

/* 
   These checks are used to ensure that the structure passed is correct.
   Both in type and that another function has not overstepped its bounds.
 */
#define MAP_PRE_CHECK (13300)
#define MAP_POST_CHECK (3600)
#define MAPENTITY_PRE_CHECK (320)
#define MAPENTITY_POST_CHECK (7936)

struct dm_map *dm_alloc_map(int x_sz, int y_sz) {
    if (x_sz < 2) return NULL;
    if (y_sz < 2) return NULL;

    int sz = sizeof(struct dm_map) + ( (x_sz) * y_sz * sizeof(*(((struct dm_map*)0)->map)) ) ;
    struct dm_map *map = malloc(sz);
    if (map == NULL) return NULL;
    memset(map, 0x0, sz);

    map->map_pre = MAP_PRE_CHECK;
    map->map_post = MAP_POST_CHECK;
    map->size = cd_create(x_sz, y_sz);
    map->seed = 0;

    dm_clear_map_unsafe(map);

    return map;
}

bool dm_free_map(struct dm_map *map) {
    if (dm_verify_map(map) == false) return false;

    coord_t c = cd_create(0,0);
    for (c.x = 0; c.x < map->size.x; c.x++) {
        for (c.y = 0; c.y < map->size.y; c.y++) {
            inv_exit(dm_get_map_me(&c,map)->inventory);
        }
    }

    free(map);
    return true;
}

/* 
   This sets the map to default values.
 */
static bool dm_clear_map_unsafe(struct dm_map *map) {
    coord_t c = cd_create(0,0);
    for (c.x = 0; c.x < map->size.x; c.x++) {
        for (c.y = 0; c.y < map->size.y; c.y++) {
            dm_get_map_me(&c,map)->map_entity_pre = MAPENTITY_PRE_CHECK;
            dm_get_map_me(&c,map)->map_entity_post = MAPENTITY_POST_CHECK;

            dm_get_map_me(&c,map)->pos = c;
            dm_get_map_me(&c,map)->in_sight = false;
            dm_get_map_me(&c,map)->visible = false;
            dm_get_map_me(&c,map)->discovered = false;
            dm_get_map_me(&c,map)->light_level = 0;
            dm_get_map_me(&c,map)->monster = NULL;
            dm_get_map_me(&c,map)->icon_override = -1;
            dm_get_map_me(&c,map)->icon_attr_override = -1;
            dm_get_map_me(&c,map)->test_var = 0;

            if (dm_get_map_me(&c,map)->inventory != NULL) inv_exit(dm_get_map_me(&c,map)->inventory);
            dm_get_map_me(&c,map)->inventory = inv_init(inv_loc_tile);

            /*
                HACK: This is a hack to give map squares items based on their attributes.
                I should find a better place for this.
             */
            if (dm_get_map_tile(&c,map) != NULL) {
                if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&c,map), TILE_ATTR_LIGHT_SOURCE) ) {
                    struct itm_item *i = itm_create(IID_FIXED_LIGHT);
                    if (itm_insert_item(i, map, &c) == false) {
                        itm_destroy(i);
                        lg_debug("light failed at (%d,%d)", c.x,c.y);
                    }
                    else {
                        i->specific.tool.lit = true;
                        lg_debug("light at (%d,%d)", c.x,c.y);
                    }
                }
            }
        }
    }
    return true;
}

/* 
   As with every verify function, check if the 
   intergrity of the passed struct is correct.
 */
bool dm_verify_map(struct dm_map *map) {
    assert(map != NULL);
    assert(map->map_pre == MAP_PRE_CHECK);
    assert(map->map_post == MAP_POST_CHECK);
    assert(map->size.x > 2);
    assert(map->size.y > 2);
    assert(map->map != NULL);

    coord_t c = cd_create(0,0);
    struct dm_map_entity *me = dm_get_map_me(&c, map);
    assert(me->map_entity_pre = MAPENTITY_PRE_CHECK);
    assert(me->map_entity_post = MAPENTITY_POST_CHECK);

    c = cd_create(map->size.x -1, map->size.y -1);
    me = dm_get_map_me(&c, map);
    assert(me->map_entity_pre = MAPENTITY_PRE_CHECK);
    assert(me->map_entity_post = MAPENTITY_POST_CHECK);

    return true;
}

bool dm_print_map(struct dm_map *map) {
    if (dm_verify_map(map) == false) return false;

    coord_t c;
    for (c.y = 0; c.y < map->size.y; c.y++) {
        for (c.x = 0; c.x < map->size.x; c.x++) {
            putchar(dm_get_map_tile(&c,map)->icon);
        }
        putchar('\n');
    }
    putchar('\n');

    return true;
}

bool dm_tile_instance(struct dm_map *map, enum tile_types tt, int instance, coord_t *pos) {
    if (dm_verify_map(map) == false) return false;

    coord_t c;
    for (c.x = 0; c.x < map->size.x; c.x++) {
        for (c.y = 0; c.y < map->size.y; c.y++) {
            if (dm_get_map_tile(&c,map)->type == tt ) {
                instance--;
                if (instance <= 0) {
                    *pos = c;
                    return true;
                }
            }
        }
    }
    return false;
}

bool dm_populate_map(struct dm_map *map, struct random *r, uint32_t monster_chance, uint32_t item_chance, int level) {
    if (dm_verify_map(map) == false) return false;
    if (r == NULL) return false;
    coord_t c;
    int idx;

    int nogo_radius = 20;

    for (int xi = 0; xi < map->size.x; xi++) {
        for (int yi = 0; yi < map->size.y; yi++) {
            c = cd_create(xi,yi);
            if (cd_pyth(&map->stair_up, &c) <= nogo_radius) continue; /* no npc's too close to the start */

            if ( (random_int32(r) % 10000) <= (monster_chance/100)+1 ) {
                uint32_t leader = 0;
                int msr_cnt = random_int32(r) % 20;
                for (int i = 0; i < msr_cnt; i++) {
                    idx = msr_spawn(random_float(r), level, map->type);
                    coord_t cp = sgt_scatter(map, r, &c, 10);

                    if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&cp,map), TILE_ATTR_TRAVERSABLE) == true) {
                        if (dm_get_map_me(&cp,map)->monster == NULL) {
                            struct msr_monster *monster = msr_create(idx);
                            msr_give_items(monster, level, r);

                            msr_insert_monster(monster, map, &c);
                            ai_monster_init(monster, leader);
                            if (leader == 0) {
                                leader = monster->uid;
                                lg_debug("created swarm leader at (%d,%d)", cp.x, cp.y);
                            }
                            else lg_debug("created swarm member at (%d,%d)", cp.x, cp.y); 
                        }
                    }
                }
            }
            else if ( (random_int32(r) % 10000) <= monster_chance) {
                if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&c,map), TILE_ATTR_TRAVERSABLE) == true) {
                    idx = msr_spawn(random_float(r), level, map->type);
                    struct msr_monster *monster = msr_create(idx);
                    msr_give_items(monster, level, r);

                    msr_insert_monster(monster, map, &c);
                    ai_monster_init(monster, 0);
                }
            }
            else if ( (random_int32(r) % 10000) <= item_chance) {
                if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&c,map), TILE_ATTR_TRAVERSABLE) == true) {
                    int item_level = level;
                    if (random_float(r) > 0.95) item_level += 1;
                    if (random_float(r) > 0.99) item_level += 1;
                    idx = itm_spawn(random_float(r), item_level, ITEM_GROUP_ANY);
                    struct itm_item *item = itm_create(idx);

                    itm_insert_item(item, map, &c);
                }
            }
        }
    }

    return true;
}

/* 
   Here we add stairs, supports only 2 at a time, for now.
   randomly selects a tile, puts down the up stairs,
   then tries a few times untill the distance is satisfying 
   or untill we run out of tries.

   then it places the stairs.
 */
static void dm_add_stairs(struct dm_map *map, struct random *r) {
    if (dm_verify_map(map) == false) return;

    struct tl_tile **tile_up = NULL;
    struct tl_tile **tile_down = NULL;
    struct tl_tile **tile_down_temp = NULL;
    int i = 0;
    int large_num = 10000;
    int target_distance = map->size.x * 0.8f;
    
    int last_distance = 0;
    coord_t up = cd_create(0,0);
    coord_t down = cd_create(0,0);
    coord_t down_temp = cd_create(0,0);

    while (tile_up == NULL || tile_down == NULL) {
        coord_t c;
        c.x = random_int32(r) % map->size.x;
        c. y = random_int32(r) % map->size.y;
        i++;

        if (dm_get_map_tile(&c,map)->type == TILE_TYPE_FLOOR ) {
            if (tile_up == NULL) {
                tile_up = &dm_get_map_me(&c,map)->tile;
                up = c;
            }
            else if (tile_down == NULL) {
                int xdiff = (abs(c.x - up.x) );
                int ydiff = (abs(c.y - up.y) );
                int dist = 0;
                if ( (dist = pyth(xdiff, ydiff)) > target_distance) {
                    tile_down = &dm_get_map_me(&c,map)->tile;
                    down = c;
                }
                else if (dist > last_distance) {
                    tile_down_temp = &dm_get_map_me(&c,map)->tile;
                    last_distance = dist;
                    down_temp = c;
                }
            }
        }

        if ( (tile_down == NULL) && (i > large_num) ) {
            tile_down = tile_down_temp;
            i = 0;
            down = down_temp;
        }
    }
    *tile_up = ts_get_tile_type(TILE_TYPE_STAIRS_UP);
    *tile_down = ts_get_tile_type(TILE_TYPE_STAIRS_DOWN);

    map->stair_up = up;
    map->stair_down = down;
}

bool dm_clear_map(struct dm_map *map) {
    if (dm_verify_map(map) == false) return false;

    return dm_clear_map_unsafe(map);
}

/*
    resets visibility values.
 */
bool dm_clear_map_visibility(struct dm_map *map, coord_t *start, coord_t *end) {
    if (dm_verify_map(map) == false) return false;

    if (cd_within_bound(start, &map->size) == false) return false;
    if (end->x > map->size.x) return false;
    if (end->y > map->size.y) return false;
    if (end->x+start->x > map->size.x) return false;
    if (end->y+start->y > map->size.y) return false;

    coord_t c = cd_add(start, end);
    for (c.x = start->x; c.x < end->x; c.x++) {
        for (c.y = start->y; c.y < end->y; c.y++) {
            dm_get_map_me(&c,map)->in_sight = false;
            dm_get_map_me(&c,map)->visible = false;
            dm_get_map_me(&c,map)->light_level = 0;
            //dm_get_map_me(&c,map)->test_var = 0;
            dm_get_map_me(&c,map)->icon_override = -1;
            dm_get_map_me(&c,map)->icon_attr_override = -1;
        }
    }
    return true;
}

static bool dm_tunnel(struct dm_map *map, struct random *r, coord_t *start, coord_t *end, struct tl_tile *tl) {
    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(start, &map->size) == false) return false;
    if (cd_within_bound(end, &map->size) == false) return false;
    if (tl == NULL) return false;

    coord_t prev = { .x = start->x, .y = start->y, };
    bool tunnel_done = false;
    while (!tunnel_done) {
        coord_t delta     = cd_delta(end, &prev);
        coord_t delta_abs = cd_delta_abs(end, &prev);
        
        int xd = 0;
        int yd = 0;
        int xmod = (delta.x >= 0) ? 1: -1;
        int ymod = (delta.y >= 0) ? 1: -1;
        
        int roll = random_int32(r) % 100;
        if (roll < 50) {
            if (delta_abs.x >= delta_abs.y) {
                xd = 1 * xmod;
            }
            else if (delta_abs.x <= delta_abs.y) {
                yd = 1 * ymod;
            }
        }
        else {
            roll = random_int32(r) % 4;
            if (roll < 30) {
                xd = 1 * xmod;
            }
            else if (roll < 60) {
                yd = 1 * ymod;
            }
            else if (roll < 80) {
                xd = 1 * -xmod;
            }
            else if (roll < 100) {
                yd = 1 * -ymod;
            }
        }

        coord_t next = { .x = prev.x +xd, .y = prev.y +yd, };
        if (cd_within_bound(&next, &map->size) ) {
            dm_get_map_me(&next, map)->tile = tl;
            prev.x = next.x;
            prev.y = next.y;

            if (cd_equal(&prev, end) ) tunnel_done = true;
        }
    }
    return true;
}

static bool dm_has_floors(struct dm_map *map) {
    coord_t c;

    for (c.x = 0; c.x < map->size.x; c.x++) {
        for (c.y = 0; c.y < map->size.y; c.y++) {
            if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&c, map),TILE_ATTR_TRAVERSABLE) ) return true;
        }
    }

    return false;
}

static bool dm_get_tunnel_path(struct dm_map *map, struct pf_context *pf_ctx, struct random *r) {
    if (dm_verify_map(map) == false) return false;

    bool retval = false;

    /* get a coords from a place we did not reach with our flooding, nearest to the stairs*/
    coord_t nftl;
    if (pf_get_non_flooded_tile(pf_ctx, &map->stair_up, &nftl) == true) {

        /* get coords of a place we DID reach, so we can connect to that*/
        coord_t ftl;
        if (pf_get_closest_flooded_tile(pf_ctx, &nftl, &ftl) == true) {

            /* get the closest non-flooded tile to the flooded tile we just found. */
            coord_t nftl2;
            if (pf_get_non_flooded_tile(pf_ctx, &ftl, &nftl2) == true) {
                /* Tunnel our way to there.. */
                struct tl_tile *tl = ts_get_tile_specific(TILE_ID_CONCRETE_FLOOR);
                if (dm_tunnel(map, r, &ftl, &nftl2, tl) ) return true;
            }
        }
    }

    return retval;
}

static void dm_add_lights(struct dm_map *map, struct random *r) {
    for (int x = 1; x < map->size.x; x++) {
        for (int y = 1; y < map->size.y; y++) {
            coord_t point = { .x = x, .y = y, };

            struct dm_map_entity *me = dm_get_map_me(&point, map);
            if (me->tile->type == TILE_TYPE_WALL ) {
                if(random_int32(r)%100 < 1) {
                    me->tile = ts_get_tile_specific(TILE_ID_CONCRETE_WALL_LIT);
                    lg_debug("light at (%d,%d)", x,y);
                }
            }
        }
    }

}

bool dm_generate_map(struct dm_map *map, enum dm_dungeon_type type, int level, unsigned long seed, bool populate) {
    if (dm_verify_map(map) == false) return false;

    map->seed = seed;
    map->type = type;
    map->threat_lvl = level;

    lg_debug("generating map with seed \'%lu\', type \'%d\' and threat_lvl \'%d\'", seed, type, level);

    /* Create an non-destructable border wall around the map */
    coord_t c;
    for (c.x = 0, c.y = 0; c.x < map->size.x; c.x++) dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_BORDER_WALL);
    for (c.y = 0, c.x = 0; c.y < map->size.y; c.y++) dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_BORDER_WALL);
    for (c.x = 0, c.y = map->size.y-1; c.x < map->size.x; c.x++) dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_BORDER_WALL);
    for (c.y = 0, c.x = map->size.x-1; c.y < map->size.y; c.y++) dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_BORDER_WALL);

    coord_t ul = { .x = 1, .y = 1,};
    coord_t dr = { .x = map->size.x-1, .y = map->size.y-1, };
    struct random *r = random_init_genrand(seed);
    switch(type) {
        case DUNGEON_TYPE_CAVE:
            cave_generate_map(map, r, type, &ul, &dr);
            break;
        default:
        case DUNGEON_TYPE_PLAIN:
            dm_generate_map_plain(map, r, type, &ul, &dr);
            break;
    }
    assert(dm_has_floors(map) );

    dm_add_lights(map, r);
    dm_add_stairs(map, r);
    dm_clear_map(map);

    coord_t start = { .x = map->stair_up.x, .y = map->stair_up.y};
    coord_t end = { .x = map->stair_down.x, .y = map->stair_down.y};

    bool map_is_good = false;
    struct pf_context *pf_ctx = NULL;

    int i = 0;
    for (i = 0; i < 1000 && (map_is_good == false); i++) {
        /*
           We flood the map and rescue nonflooded segments untill we can find 
           no more non-flooded tiles. This takes a long time though, 
           and can probably be optimised.
         */
        if (aiu_generate_dijkstra(&pf_ctx, map, &start, 0) ) {
            if (pf_calculate_reachability(pf_ctx) == true) {
                map_is_good = true;
                assert(pf_calculate_path(pf_ctx, &start, &end, NULL) > 1);
            }
            else {
                dm_get_tunnel_path(map, pf_ctx, r);
            }

        }
    }

    assert(map_is_good == true);
    lg_debug("Map is completly reachable in %d tries", i);

    pf_exit(pf_ctx);

    if (populate) dm_populate_map(map, r, 100, 5, level);
    random_exit(r);

    return true;
}

