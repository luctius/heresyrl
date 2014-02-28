#pragma once
#ifndef DUNGEON_CREATOR_H_
#define DUNGEON_CREATOR_H_

#include <stdint.h>
#include <stdbool.h>

#include "heresyrl_def.h"
#include "tiles.h"

#define SD_GET_INDEX(x,y, map_ptr) map_ptr->map[((x) * map_ptr->y_sz) + (y)]
#define SD_GET_INDEX_TYPE(x,y, map_ptr) SD_GET_INDEX(x,y,map_ptr).tile->type
#define SD_GET_INDEX_ICON(x,y, map_ptr) SD_GET_INDEX(x,y,map_ptr).tile->icon

enum dc_dungeon_type {
    DC_DUNGEON_TYPE_SIMPLE,
    DC_DUNGEON_TYPE_CAVE,
};

struct dc_map_entity {
    char type;
    bool in_sight;
    bool visible;
    bool discovered;
    uint8_t light_level;
    uint8_t general_var;
    struct tl_tile *tile;
    struct msr_monster *monster;
    struct itm_items *item;
};

struct dc_map {
    int x_sz;
    int y_sz;
    unsigned int seed;

    struct dc_map_entity map[];
};

struct dc_map *dc_alloc_map(int x_sz, int y_sz);
int dc_free_map(struct dc_map *map);

int dc_print_map(struct dc_map *map);
bool dc_generate_map(struct dc_map *map, enum dc_dungeon_type type, int level, unsigned long seed);
bool dc_tile_instance(struct dc_map *map, enum tile_types tt, int instance, int *xpos, int *ypos);
void dc_clear_map_visibility(struct dc_map *map, int sx, int sy, int ex, int ey);

#endif /*DUNGEON_CREATOR_H_*/
