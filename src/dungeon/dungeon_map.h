#ifndef DUNGEON_MAP_H
#define DUNGEON_MAP_H

#include <stdint.h>
#include <stdbool.h>

#include "heresyrl_def.h"
#include "coord.h"

enum dm_dungeon_type {
    DM_DUNGEON_TYPE_SIMPLE,
    DM_DUNGEON_TYPE_CAVE,
};

struct dm_map_entity {
    uint32_t map_entity_pre;

    coord_t pos;
    char type;
    char icon_override;
    int icon_attr_override;
    bool in_sight;
    bool visible;
    bool discovered;
    int light_level;
    struct tl_tile *tile;
    struct msr_monster *monster;
    struct inv_inventory *inventory;
    int test_var;

    uint32_t map_entity_post;
};

struct dm_map {
    uint32_t map_pre;

    coord_t size;
    unsigned long seed;
    enum dm_dungeon_type type;
    int threat_lvl;

    coord_t stair_up;
    coord_t stair_down;

    uint32_t map_post;

    struct dm_map_entity map[];
};

inline struct dm_map_entity *dm_get_map_me(coord_t *c, struct dm_map *map) {
    if (cd_within_bound(c, &map->size) == false) return NULL;
    return &map->map[((c->x) * (map)->size.y) + (c->y)];
}

inline struct tl_tile *dm_get_map_tile(coord_t *c, struct dm_map *map) {
    if (cd_within_bound(c, &map->size) == false) return NULL;
    return map->map[((c->x) * (map)->size.y) + (c->y)].tile;
}

struct dm_map *dm_alloc_map(int x_sz, int y_sz);
bool dm_free_map(struct dm_map *map);
bool dm_verify_map(struct dm_map *map);

bool dm_print_map(struct dm_map *map);
bool dm_generate_map(struct dm_map *map, enum dm_dungeon_type type, int level, unsigned long seed);
bool dm_tile_instance(struct dm_map *map, enum tile_types tt, int instance, coord_t *pos);
bool dm_clear_map_visibility(struct dm_map *map, coord_t *start, coord_t *end);
bool dm_clear_map(struct dm_map *map);

#endif /* DUNGEON_MAP_H */
