#ifndef DUNGEON_CREATOR_H_
#define DUNGEON_CREATOR_H_

#include <stdint.h>
#include <stdbool.h>

#include "heresyrl_def.h"
#include "coord.h"

enum dc_dungeon_type {
    DC_DUNGEON_TYPE_SIMPLE,
    DC_DUNGEON_TYPE_CAVE,
};

struct dc_map_entity {
    coord_t pos;
    char type;
    bool in_sight;
    bool visible;
    bool discovered;
    uint8_t light_level;
    uint8_t general_var;
    struct tl_tile *tile;
    struct msr_monster *monster;
    struct itm_item *item;
};

struct dc_map {
    coord_t size;
    unsigned int seed;

    coord_t stair_up;
    coord_t stair_down;

    struct dc_map_entity map[];
};

inline struct dc_map_entity *sd_get_map_me(coord_t *c, struct dc_map *map) {
    return &map->map[((c->x) * (map)->size.y) + (c->y)];
}

inline struct tl_tile *sd_get_map_tile(coord_t *c, struct dc_map *map) {
    return map->map[((c->x) * (map)->size.y) + (c->y)].tile;
}

struct dc_map *dc_alloc_map(int x_sz, int y_sz);
int dc_free_map(struct dc_map *map);

int dc_print_map(struct dc_map *map);
bool dc_generate_map(struct dc_map *map, enum dc_dungeon_type type, int level, unsigned long seed);
bool dc_tile_instance(struct dc_map *map, enum tile_types tt, int instance, coord_t *pos);
bool dc_clear_map_visibility(struct dc_map *map, coord_t *start, coord_t *end);

#endif /*DUNGEON_CREATOR_H_*/
