/*
    This file is part of heresyRL.

    heresyRL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    heresyRL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with heresyRL.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DUNGEON_MAP_H
#define DUNGEON_MAP_H

#include <stdint.h>
#include <stdbool.h>

#include "heresyrl_def.h"
#include "coord.h"

enum dm_dungeon_type {
    DUNGEON_TYPE_NONE,
    DUNGEON_TYPE_PLAIN,
    DUNGEON_TYPE_TUNNEL,
    DUNGEON_TYPE_CAVE,
    DUNGEON_TYPE_ALL,
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
    struct ground_effect *effect;
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
bool dm_generate_map(struct dm_map *map, enum dm_dungeon_type type, int level, unsigned long seed, bool populate);
bool dm_tile_instance(struct dm_map *map, enum tile_types tt, int instance, coord_t *pos);
bool dm_clear_map_visibility(struct dm_map *map, coord_t *start, coord_t *end);
bool dm_clear_map(struct dm_map *map);

void dm_process_tiles(struct dm_map *map);
bool dm_tile_enter(struct dm_map *map, coord_t *point, struct msr_monster *monster, coord_t *prev);
bool dm_tile_exit(struct dm_map *map, coord_t *point, struct msr_monster *monster, coord_t *next);

#endif /* DUNGEON_MAP_H */
