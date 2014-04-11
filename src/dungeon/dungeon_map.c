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

#include "dungeon_cave.h"
#include "tiles.h"
#include "random.h"
#include "ai/ai_utils.h"
#include "ai/pathfinding.h"
#include "inventory.h"
#include "items/items.h"
#include "items/items_static.h"

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

static bool dm_generate_map_simple(struct dm_map *map, struct random *r, enum dm_dungeon_type type, int level) {
    if (dm_verify_map(map) == false) return false;
    FIX_UNUSED(r);
    FIX_UNUSED(type);
    FIX_UNUSED(level);

    coord_t c;
    for (c.x = 0; c.x < map->size.x; c.x++) {
        for (c.y = 0; c.y < map->size.y; c.y++) {
            if (c.y == 0 || c.y == map->size.y -1) dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_BORDER_WALL);
            else if (c.x == 0 || c.x == map->size.y -1) dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_BORDER_WALL);
            else dm_get_map_me(&c,map)->tile = ts_get_tile_type(TILE_TYPE_FLOOR);
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

/*
   This callback is used to create a path through rock and 
   rescue a locked in section of the map.
   Hence it will only return a PF_BLOCKED when it has 
   reached the sides of the map.
 */
unsigned int tunnel_callback(void *vmap, coord_t *coord) {
    if (vmap == NULL) return PF_BLOCKED;
    if (coord == NULL) return PF_BLOCKED;
    struct dm_map *map = (struct dm_map *) vmap;

    if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(coord, map),TILE_ATTR_BORDER) == true) return PF_BLOCKED;
    return 1;
}

/*
   Given a path to tunnel, it will replace the original tile 
   pointer with the given one, thus tunneling a path throught walls.
 */
static bool dm_tunnel(struct dm_map *map, coord_t plist[], int plsz, struct tl_tile *tl) {
    if (plist == NULL) return false;
    if (tl == NULL) return false;
    if (plsz == 0) return false;

    /* copy the given tile over the path. */
    for (int i = 0; i < plsz; i++) {
        dm_get_map_me(&plist[i], map)->tile = tl;
    }
    return true;
}

static bool dm_get_tunnel_path(struct dm_map *map, struct pf_context *pf_ctx) {
    if (dm_verify_map(map) == false) return false;

    bool retval = false;

    /* get a coords from a place we did not reach with our flooding*/
    coord_t nftl;
    if (pf_get_non_flooded_tile(pf_ctx, &nftl) == true) {

        /* get coords of a place we DID reach, so we can connect to that*/
        coord_t ftl;
        if (pf_get_closest_flooded_tile(pf_ctx, &nftl, &ftl) == true) {

            /*save current, probably generic, callback*/
            struct pf_settings *set = pf_get_settings(pf_ctx);
            void *tcbk = set->pf_traversable_callback;

            /*set our tunneling callback*/
            set->pf_traversable_callback = tunnel_callback;

            /*run the pathfinding algorithm*/
            if (pf_astar_map(pf_ctx, &nftl, &ftl) == true) {
                int sz = 0;

                /*retreive the path */
                coord_t *plist = NULL;
                if ( (sz = pf_calculate_path (pf_ctx, &nftl, &ftl, &plist) ) > 0) {

                    /* HACK:
                       this is the tile we will copy.
                       i assume the map does only contain 
                       generic tiles...
                     */
                    struct tl_tile *tl = dm_get_map_tile(&ftl, map);

                    if (dm_tunnel(map, plist, sz, tl) == true) {
                        retval = true;
                    }
                    free(plist);
                }
            }

            /* restore callback*/
            set->pf_traversable_callback = tcbk;
        }
    }

    return retval;
}

bool dm_generate_map(struct dm_map *map, enum dm_dungeon_type type, int level, unsigned long seed) {
    if (dm_verify_map(map) == false) return false;

    map->seed = seed;
    map->type = type;
    map->threat_lvl = level;

    lg_debug("generating map with seed \'%d\', type \'%d\' adn threat_lvl \'%d\'", seed, type, level);

    struct random *r = random_init_genrand(seed);
    switch(type) {
        case DM_DUNGEON_TYPE_CAVE:
            cave_generate_map(map, r, type, level);
            break;
        default:
            dm_generate_map_simple(map, r, type, level);
            break;
    }

    dm_add_stairs(map, r);
    dm_clear_map(map);

    coord_t start = { .x = map->stair_up.x, .y = map->stair_up.y};
    coord_t end = { .x = map->stair_down.x, .y = map->stair_down.y};

    bool map_is_good = false;
    struct pf_context *pf_ctx = NULL;

    while (map_is_good == false)
    {
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
                dm_get_tunnel_path(map, pf_ctx);
            }

        }
    }

    pf_exit(pf_ctx);
    random_exit(r);

    return true;
}

