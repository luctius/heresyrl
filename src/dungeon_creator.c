#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "random.h"
#include "heresyrl_def.h"
#include "dungeon_creator.h"
#include "dungeon_cave.h"
#include "pathfinding.h"

struct dc_map *dc_alloc_map(int x_sz, int y_sz) {
    if (x_sz < 2) return NULL;
    if (y_sz < 2) return NULL;

    int sz = sizeof(struct dc_map) + ( (x_sz) * y_sz * sizeof(*(((struct dc_map*)0)->map)) ) ;
    struct dc_map *map = malloc(sz);
    if (map == NULL) return NULL;

    map->size = cd_create(x_sz, y_sz);
    map->seed = 0;
    return map;
}

int dc_free_map(struct dc_map *map) {
    if (map == NULL) return EXIT_SUCCESS;
    free(map);
    return EXIT_SUCCESS;
}

int dc_print_map(struct dc_map *map) {
    if (map == NULL) return EXIT_FAILURE;
    if (map->size.x < 2) return EXIT_FAILURE;
    if (map->size.y < 2) return EXIT_FAILURE;
    if (map->map == NULL) return EXIT_FAILURE;

    coord_t c;
    for (c.y = 0; c.y < map->size.y; c.y++) {
        for (c.x = 0; c.x < map->size.x; c.x++) {
            putchar(SD_GET_INDEX_ICON(&c,map));
        }
        putchar('\n');
    }
    putchar('\n');

    return EXIT_SUCCESS;
}

bool dc_tile_instance(struct dc_map *map, enum tile_types tt, int instance, coord_t *pos) {
    coord_t c;
    for (c.x = 0; c.x < map->size.x; c.x++) {
        for (c.y = 0; c.y < map->size.y; c.y++) {
            if (SD_GET_INDEX_TYPE(&c,map) == tt ) {
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
    if (map == NULL) return false;
    if (map->size.x < 2) return false;
    if (map->size.y < 2) return false;
    if (map->map == NULL) return false;

    coord_t c;
    for (c.x = 0; c.x < map->size.x; c.x++) {
        for (c.y = 0; c.y < map->size.y; c.y++) {
            if (c.y == 0 || c.y == map->size.y -1) SD_GET_INDEX(&c,map).tile = ts_get_tile_specific(TILE_ID_BORDER_WALL);
            else if (c.x == 0 || c.x == map->size.y -1) SD_GET_INDEX(&c,map).tile = ts_get_tile_specific(TILE_ID_BORDER_WALL);
            else SD_GET_INDEX(&c,map).tile = ts_get_tile_type(TILE_TYPE_FLOOR);
        }
    }
    return true;
}

static void dc_add_stairs(struct dc_map *map, struct random *r) {
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
        c.x = random_genrand_int32(r) % map->size.x;
        c. y = random_genrand_int32(r) % map->size.y;
        i++;

        if (SD_GET_INDEX_TYPE(&c,map) == TILE_TYPE_FLOOR ) {
            if (tile_up == NULL) {
                tile_up = &SD_GET_INDEX(&c,map).tile;
                up = c;
            }
            else if (tile_down == NULL) {
                int xdiff = (abs(c.x - up.x) );
                int ydiff = (abs(c.y - up.y) );
                int dist = 0;
                if ( (dist = pyth(xdiff, ydiff)) > target_distance) {
                    tile_down = &SD_GET_INDEX(&c,map).tile;
                    down = c;
                }
                else if (dist > last_distance) {
                    tile_down_temp = &SD_GET_INDEX(&c,map).tile;
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

static bool dc_clear_map(struct dc_map *map) {
    if (map == NULL) return false;
    if (map->size.x < 2) return false;
    if (map->size.y < 2) return false;
    if (map->map == NULL) return false;

    coord_t c = cd_create(0,0);
    for (c.x = 0; c.x < map->size.x; c.x++) {
        for (c.y = 0; c.y < map->size.y; c.y++) {
            SD_GET_INDEX(&c,map).pos = c;
            SD_GET_INDEX(&c,map).in_sight = false;
            SD_GET_INDEX(&c,map).visible = false;
            SD_GET_INDEX(&c,map).discovered = false;
            SD_GET_INDEX(&c,map).light_level = 0;
            SD_GET_INDEX(&c,map).general_var = 0;
            SD_GET_INDEX(&c,map).monster = NULL;
            SD_GET_INDEX(&c,map).item = NULL;
        }
    }
    return true;
}

bool dc_clear_map_visibility(struct dc_map *map, coord_t *start, coord_t *end) {
    if (map == NULL) return false;
    if (map->size.x < 2) return false;
    if (map->size.y < 2) return false;
    if (map->map == NULL) return false;
    if (cd_within_bound(start, &map->size) == false) return false;
    if (cd_within_bound(end, &map->size) == false) return false;

    coord_t c = cd_add(start, end);
    if (cd_within_bound(&c, &map->size) == false) return false;

    for (c.x = start->x; c.x < end->x; c.x++) {
        for (c.y = start->y; c.y < end->y; c.y++) {
            SD_GET_INDEX(&c,map).in_sight = false;
            SD_GET_INDEX(&c,map).visible = false;
            SD_GET_INDEX(&c,map).light_level = 0;
        }
    }
    return true;
}

static unsigned int dc_traversable_callback(void *vmap, struct pf_coord *coord) {
    if (vmap == NULL) return PF_BLOCKED;
    if (coord == NULL) return PF_BLOCKED;
    struct dc_map *map = (struct dc_map *) vmap;

    unsigned int cost = PF_BLOCKED;
    coord_t c = cd_create(coord->x, coord->y);
    if (TILE_HAS_ATTRIBUTE(SD_GET_INDEX(&c, map).tile,TILE_ATTR_TRAVERSABLE) == true) {
        cost = SD_GET_INDEX(&c, map).tile->movement_cost;
    }
    if (TILE_HAS_ATTRIBUTE(SD_GET_INDEX(&c, map).tile,TILE_ATTR_BORDER) == true) cost = PF_BLOCKED;

    return cost;
}

bool dc_generate_map(struct dc_map *map, enum dc_dungeon_type type, int level, unsigned long seed) {
    if (map == NULL) return false;
    map->seed = seed;

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

    struct pf_context *pf_ctx = pf_init();

    struct pf_settings pf_set = { 
        .max_traversable_cost = 2,
        .map_start = { 
            .x = 0, 
            .y = 0, 
        }, 
        .map_end = {
            .x = map->size.x,
            .y = map->size.y,
        },
        .map = map,
        .pf_traversable_callback = dc_traversable_callback,
    };
    struct pf_coord start = { .x = map->stair_up.x, .y = map->stair_up.y};
    struct pf_coord end = { .x = map->stair_down.x, .y = map->stair_down.y};

    if (pf_flood_map(pf_ctx, &pf_set, &start) ) {
        pf_calculate_reachability(pf_ctx);

        if (pf_calculate_path(pf_ctx, &start, &end, NULL) > 1) {
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "dc", "Stairs reachable.");
        }
        else {
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "dc", "Stairs not reachable.");
            dc_generate_map(map, type, level, random_genrand_int32(r) );
        }
    }

    pf_exit(pf_ctx);
    random_exit(r);
    return true;
}

