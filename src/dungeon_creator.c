#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <ncurses.h>

#include "heresyrl_def.h"
#include "dungeon_creator.h"
#include "dungeon_cave.h"
#include "tiles.h"
#include "ai_utils.h"
#include "random.h"
#include "pathfinding.h"
#include "inventory.h"
#include "items.h"
#include "items_static.h"

extern inline struct dc_map_entity *sd_get_map_me(coord_t *c, struct dc_map *map);
extern inline struct tl_tile *sd_get_map_tile(coord_t *c, struct dc_map *map);
static bool dc_clear_map_unsafe(struct dc_map *map);

#define MAP_PRE_CHECK (13300)
#define MAP_POST_CHECK (3600)
#define MAPENTITY_PRE_CHECK (320)
#define MAPENTITY_POST_CHECK (7936)

struct dc_map *dc_alloc_map(int x_sz, int y_sz) {
    if (x_sz < 2) return NULL;
    if (y_sz < 2) return NULL;

    int sz = sizeof(struct dc_map) + ( (x_sz) * y_sz * sizeof(*(((struct dc_map*)0)->map)) ) ;
    struct dc_map *map = malloc(sz);
    if (map == NULL) return NULL;

    map->map_pre = MAP_PRE_CHECK;
    map->map_post = MAP_POST_CHECK;
    map->size = cd_create(x_sz, y_sz);
    map->seed = 0;

    dc_clear_map_unsafe(map);

    return map;
}

bool dc_free_map(struct dc_map *map) {
    if (dc_verify_map(map) == false) return false;

    coord_t c = cd_create(0,0);
    for (c.x = 0; c.x < map->size.x; c.x++) {
        for (c.y = 0; c.y < map->size.y; c.y++) {
            inv_exit(sd_get_map_me(&c,map)->inventory);
        }
    }

    free(map);
    return true;
}

static bool dc_clear_map_unsafe(struct dc_map *map) {
    coord_t c = cd_create(0,0);
    for (c.x = 0; c.x < map->size.x; c.x++) {
        for (c.y = 0; c.y < map->size.y; c.y++) {
            sd_get_map_me(&c,map)->map_entity_pre = MAPENTITY_PRE_CHECK;
            sd_get_map_me(&c,map)->map_entity_post = MAPENTITY_POST_CHECK;

            sd_get_map_me(&c,map)->pos = c;
            sd_get_map_me(&c,map)->in_sight = false;
            sd_get_map_me(&c,map)->visible = false;
            sd_get_map_me(&c,map)->discovered = false;
            sd_get_map_me(&c,map)->light_level = 0;
            sd_get_map_me(&c,map)->monster = NULL;
            sd_get_map_me(&c,map)->icon_override = -1;
            sd_get_map_me(&c,map)->icon_attr_override = -1;
            sd_get_map_me(&c,map)->test_var = 0;

            if (sd_get_map_me(&c,map)->inventory != NULL) inv_exit(sd_get_map_me(&c,map)->inventory);
            sd_get_map_me(&c,map)->inventory = inv_init(inv_loc_tile);

            if (sd_get_map_tile(&c,map) != NULL) {
                if (TILE_HAS_ATTRIBUTE(sd_get_map_tile(&c,map), TILE_ATTR_LIGHT_SOURCE) ) {
                    struct itm_item *i = itm_create(ITEM_ID_FIXED_LIGHT);
                    if (itm_insert_item(i, map, &c) == false) {
                        itm_destroy(i);
                    }
                    else {
                        i->specific.tool.lit = true;
                    }
                }
            }
        }
    }
    return true;
}

bool dc_verify_map(struct dc_map *map) {
    assert(map != NULL);
    assert(map->map_pre == MAP_PRE_CHECK);
    assert(map->map_post == MAP_POST_CHECK);
    assert(map->size.x > 2);
    assert(map->size.y > 2);
    assert(map->map != NULL);

    coord_t c = cd_create(0,0);
    struct dc_map_entity *me = sd_get_map_me(&c, map);
    assert(me->map_entity_pre = MAPENTITY_PRE_CHECK);
    assert(me->map_entity_post = MAPENTITY_POST_CHECK);

    c = cd_create(map->size.x -1, map->size.y -1);
    me = sd_get_map_me(&c, map);
    assert(me->map_entity_pre = MAPENTITY_PRE_CHECK);
    assert(me->map_entity_post = MAPENTITY_POST_CHECK);

    return true;
}

bool dc_print_map(struct dc_map *map) {
    if (dc_verify_map(map) == false) return false;

    coord_t c;
    for (c.y = 0; c.y < map->size.y; c.y++) {
        for (c.x = 0; c.x < map->size.x; c.x++) {
            putchar(sd_get_map_tile(&c,map)->icon);
        }
        putchar('\n');
    }
    putchar('\n');

    return true;
}

bool dc_tile_instance(struct dc_map *map, enum tile_types tt, int instance, coord_t *pos) {
    if (dc_verify_map(map) == false) return false;

    coord_t c;
    for (c.x = 0; c.x < map->size.x; c.x++) {
        for (c.y = 0; c.y < map->size.y; c.y++) {
            if (sd_get_map_tile(&c,map)->type == tt ) {
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

static bool dc_generate_map_simple(struct dc_map *map, struct random *r, enum dc_dungeon_type type, int level) {
    if (dc_verify_map(map) == false) return false;
    FIX_UNUSED(r);
    FIX_UNUSED(type);
    FIX_UNUSED(level);

    coord_t c;
    for (c.x = 0; c.x < map->size.x; c.x++) {
        for (c.y = 0; c.y < map->size.y; c.y++) {
            if (c.y == 0 || c.y == map->size.y -1) sd_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_BORDER_WALL);
            else if (c.x == 0 || c.x == map->size.y -1) sd_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_BORDER_WALL);
            else sd_get_map_me(&c,map)->tile = ts_get_tile_type(TILE_TYPE_FLOOR);
        }
    }
    return true;
}

static void dc_add_stairs(struct dc_map *map, struct random *r) {
    if (dc_verify_map(map) == false) return;

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

        if (sd_get_map_tile(&c,map)->type == TILE_TYPE_FLOOR ) {
            if (tile_up == NULL) {
                tile_up = &sd_get_map_me(&c,map)->tile;
                up = c;
            }
            else if (tile_down == NULL) {
                int xdiff = (abs(c.x - up.x) );
                int ydiff = (abs(c.y - up.y) );
                int dist = 0;
                if ( (dist = pyth(xdiff, ydiff)) > target_distance) {
                    tile_down = &sd_get_map_me(&c,map)->tile;
                    down = c;
                }
                else if (dist > last_distance) {
                    tile_down_temp = &sd_get_map_me(&c,map)->tile;
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

bool dc_clear_map(struct dc_map *map) {
    if (dc_verify_map(map) == false) return false;

    return dc_clear_map_unsafe(map);
}

bool dc_clear_map_visibility(struct dc_map *map, coord_t *start, coord_t *end) {
    if (dc_verify_map(map) == false) return false;

    if (cd_within_bound(start, &map->size) == false) return false;
    if (end->x > map->size.x) return false;
    if (end->y > map->size.y) return false;
    if (end->x+start->x > map->size.x) return false;
    if (end->y+start->y > map->size.y) return false;

    coord_t c = cd_add(start, end);
    for (c.x = start->x; c.x < end->x; c.x++) {
        for (c.y = start->y; c.y < end->y; c.y++) {
            sd_get_map_me(&c,map)->in_sight = false;
            sd_get_map_me(&c,map)->visible = false;
            sd_get_map_me(&c,map)->light_level = 0;
            sd_get_map_me(&c,map)->test_var = 0;
            sd_get_map_me(&c,map)->icon_override = -1;
            sd_get_map_me(&c,map)->icon_attr_override = -1;
        }
    }
    return true;
}

static unsigned int dc_traversable_callback(void *vmap, coord_t *coord) {
    if (vmap == NULL) return PF_BLOCKED;
    if (coord == NULL) return PF_BLOCKED;
    struct dc_map *map = (struct dc_map *) vmap;

    unsigned int cost = PF_BLOCKED;
    coord_t c = cd_create(coord->x, coord->y);
    if (TILE_HAS_ATTRIBUTE(sd_get_map_tile(&c, map),TILE_ATTR_TRAVERSABLE) == true) {
        cost = sd_get_map_tile(&c, map)->movement_cost;
    }
    if (TILE_HAS_ATTRIBUTE(sd_get_map_tile(&c, map),TILE_ATTR_BORDER) == true) cost = PF_BLOCKED;

    return cost;
}

bool dc_generate_map(struct dc_map *map, enum dc_dungeon_type type, int level, unsigned long seed) {
    if (dc_verify_map(map) == false) return false;

    map->seed = seed;
    map->type = type;
    map->threat_lvl = level;

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "dc", "generating map with seed \'%d\', type \'%d\' adn threat_lvl \'%d\'", seed, type, level);

    struct random *r = random_init_genrand(seed);
    switch(type) {
        case DC_DUNGEON_TYPE_CAVE:
            cave_generate_map(map, r, type, level);
            break;
        default:
            dc_generate_map_simple(map, r, type, level);
            break;
    }

    dc_add_stairs(map, r);
    dc_clear_map(map);

    coord_t start = { .x = map->stair_up.x, .y = map->stair_up.y};
    coord_t end = { .x = map->stair_down.x, .y = map->stair_down.y};

    struct pf_context *pf_ctx = NULL;
    if (ai_generate_dijkstra(&pf_ctx, map, &start, 0) ) {
        if (pf_calculate_path(pf_ctx, &start, &end, NULL) > 1) {
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "dc", "Stairs reachable.");
        }
        else {
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "dc", "Stairs not reachable.");
            dc_generate_map(map, type, level, random_int32(r) );
        }
    }

    pf_exit(pf_ctx);
    random_exit(r);
    return true;
}

