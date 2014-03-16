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
    uint32_t map_entity_pre;

    coord_t pos;
    char type;
    bool in_sight;
    bool visible;
    bool discovered;
    int light_level;
    struct tl_tile *tile;
    struct msr_monster *monster;
    struct inv_inventory *inventory;

    uint32_t map_entity_post;
};

struct dc_map {
    uint32_t map_pre;

    coord_t size;
    unsigned int seed;
    enum dc_dungeon_type type;
    int threat_lvl;

    coord_t stair_up;
    coord_t stair_down;

    uint32_t map_post;

    struct dc_map_entity map[];
};

inline struct dc_map_entity *sd_get_map_me(coord_t *c, struct dc_map *map) {
    if (cd_within_bound(c, &map->size) == false) return NULL;
    return &map->map[((c->x) * (map)->size.y) + (c->y)];
}

inline struct tl_tile *sd_get_map_tile(coord_t *c, struct dc_map *map) {
    if (cd_within_bound(c, &map->size) == false) return NULL;
    return map->map[((c->x) * (map)->size.y) + (c->y)].tile;
}

struct dc_map *dc_alloc_map(int x_sz, int y_sz);
bool dc_free_map(struct dc_map *map);
bool dc_verify_map(struct dc_map *map);

bool dc_print_map(struct dc_map *map);
bool dc_generate_map(struct dc_map *map, enum dc_dungeon_type type, int level, unsigned long seed);
bool dc_tile_instance(struct dc_map *map, enum tile_types tt, int instance, coord_t *pos);
bool dc_clear_map_visibility(struct dc_map *map, coord_t *start, coord_t *end);
bool dc_clear_map(struct dc_map *map);

#endif /*DUNGEON_CREATOR_H_*/
