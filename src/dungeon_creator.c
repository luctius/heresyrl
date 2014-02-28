#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "random.h"
#include "dungeon_creator.h"
#include "dungeon_cave.h"

struct dc_map *dc_alloc_map(int x_sz, int y_sz) {
    if (x_sz < 2) return NULL;
    if (y_sz < 2) return NULL;

    int sz = sizeof(struct dc_map) + ( (x_sz) * y_sz * sizeof(*(((struct dc_map*)0)->map)) ) ;
    struct dc_map *map = malloc(sz);
    if (map == NULL) return NULL;

    map->x_sz = x_sz;
    map->y_sz = y_sz;
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
    if (map->x_sz < 2) return EXIT_FAILURE;
    if (map->y_sz < 2) return EXIT_FAILURE;
    if (map->map == NULL) return EXIT_FAILURE;

    for (int y = 0; y < map->y_sz; y++) {
        for (int x = 0; x < map->x_sz; x++) {
            putchar(SD_GET_INDEX_ICON(x,y,map));
        }
        putchar('\n');
    }
    putchar('\n');

    return EXIT_SUCCESS;
}

bool dc_tile_instance(struct dc_map *map, enum tile_types tt, int instance, int *xpos, int *ypos) {
    for (int x = 0; x < map->x_sz; x++) {
        for (int y = 0; y < map->y_sz; y++) {
            if (SD_GET_INDEX_TYPE(x,y,map) == tt ) {
                instance--;
                if (instance <= 0) {
                    *xpos = x;
                    *ypos = y;
                    return true;
                }
            }
        }
    }
    return false;
}

static bool dc_generate_map_simple(struct dc_map *map, struct random *r, enum dc_dungeon_type type, int level) {
    if (map == NULL) return false;
    if (map->x_sz < 2) return false;
    if (map->y_sz < 2) return false;
    if (map->map == NULL) return false;

    for (int x = 0; x < map->x_sz; x++) {
        for (int y = 0; y < map->y_sz; y++) {
            if (y == 0 || y == map->y_sz -1) SD_GET_INDEX(x,y,map).tile = ts_get_tile_specific(TILE_ID_BORDER_WALL);
            else if (x == 0 || x == map->x_sz -1) SD_GET_INDEX(x,y,map).tile = ts_get_tile_specific(TILE_ID_BORDER_WALL);
            else SD_GET_INDEX(x,y,map).tile = ts_get_tile_type(TILE_TYPE_FLOOR);
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
    int target_distance = map->x_sz * 0.8f;
    
    int last_distance = 0;
    int up_x = 0;
    int up_y = 0;

    while (tile_up == NULL || tile_down == NULL) {
        int x = random_genrand_int32(r) % map->x_sz;
        int y = random_genrand_int32(r) % map->y_sz;
        i++;

        if (SD_GET_INDEX_TYPE(x,y,map) == TILE_TYPE_FLOOR ) {
            if (tile_up == NULL) {
                tile_up = &SD_GET_INDEX(x,y,map).tile;
                up_x = x;
                up_y = y;
            }
            else if (tile_down == NULL) {
                int xdiff = (abs(x - up_x) );
                int ydiff = (abs(y - up_y) );
                int dist = 0;
                if ( (dist = pyth(xdiff, ydiff)) > target_distance) {
                    tile_down = &SD_GET_INDEX(x,y,map).tile;
                }
                else if (dist > last_distance) {
                    tile_down_temp = &SD_GET_INDEX(x,y,map).tile;
                    last_distance = dist;
                }
            }
        }

        if ( (tile_down == NULL) && (i > large_num) ) {
            tile_down = tile_down_temp;
            i = 0;
        }
    }
    *tile_up = ts_get_tile_type(TILE_TYPE_STAIRS_UP);
    *tile_down = ts_get_tile_type(TILE_TYPE_STAIRS_DOWN);
}

static void dc_clear_map(struct dc_map *map) {
    if (map == NULL) return false;
    if (map->x_sz < 2) return false;
    if (map->y_sz < 2) return false;
    if (map->map == NULL) return false;

    for (int x = 0; x < map->x_sz; x++) {
        for (int y = 0; y < map->y_sz; y++) {
            SD_GET_INDEX(x,y,map).x_pos = x;
            SD_GET_INDEX(x,y,map).y_pos = y;
            SD_GET_INDEX(x,y,map).in_sight = false;
            SD_GET_INDEX(x,y,map).visible = false;
            SD_GET_INDEX(x,y,map).discovered = false;
            SD_GET_INDEX(x,y,map).light_level = 0;
            SD_GET_INDEX(x,y,map).monster = NULL;
            SD_GET_INDEX(x,y,map).item = NULL;
        }
    }
    return true;
}

void dc_clear_map_visibility(struct dc_map *map, int sx, int sy, int ex, int ey) {
    if (map == NULL) return false;
    if (map->x_sz < 2) return false;
    if (map->y_sz < 2) return false;
    if (map->map == NULL) return false;
    if (sx < 0 || sx >= map->x_sz) return false;
    if (sy < 0 || sy >= map->y_sz) return false;
    if (sx+ex < 0 || sx+ex >= map->x_sz) return false;
    if (sy+ey < 0 || sy+ey >= map->y_sz) return false;

    for (int x = sx; x < ex; x++) {
        for (int y = sy; y < ey; y++) {
            SD_GET_INDEX(x,y,map).in_sight = false;
            SD_GET_INDEX(x,y,map).visible = false;
            SD_GET_INDEX(x,y,map).light_level = 0;
        }
    }
    return true;
}

bool dc_generate_map(struct dc_map *map, enum dc_dungeon_type type, int level, unsigned long seed) {
    if (map == NULL) return false;

    struct random *r = random_init_genrand(seed);
    switch(type) {
        case DC_DUNGEON_TYPE_CAVE:
            cave_generate_map(map, r, type, level);
            break;
        default:
            dc_generate_map_simple(map, r, type, level);
            break;
    }

    /* Check map with flooding... */
    dc_add_stairs(map, r);
    dc_clear_map(map);

    random_exit(r);
    map->seed = seed;
    return true;
}

