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

#ifndef TILES_H_
#define TILES_H_

#include <stdint.h>

#include "status_effects/status_effects.h"
#include "monster/monster.h"

#define TILE_HAS_ATTRIBUTE(tile, attr) ((tile->attributes & attr) > 0)
#define TILE_MOVEMENT_MAX (UINT8_MAX)
#define TILE_COST_DIV (10)

enum tile_attributes {
    TILE_ATTR_TRAVERSABLE    = (1<<0),
    TILE_ATTR_TRANSPARENT    = (1<<1),
    TILE_ATTR_DOOR_CLOSED    = (1<<2),
    TILE_ATTR_DOOR_OPEN      = (1<<3),
    TILE_ATTR_STAIRS_UP      = (1<<4),
    TILE_ATTR_STAIRS_DOWN    = (1<<5),
    TILE_ATTR_HAZARDOUS      = (1<<6),
    TILE_ATTR_BORDER         = (1<<7),
    TILE_ATTR_WALL           = (1<<8),
    TILE_ATTR_LIGHT_SOURCE   = (1<<9),
    TILE_ATTR_MAX,
};

enum tile_ids {
    TILE_ID_NONE,
    TILE_ID_TUNNEL_DUMMY,
    TILE_ID_BORDER_WALL,
    TILE_ID_CONCRETE_WALL,
    TILE_ID_BRASSIER,
    TILE_ID_CONCRETE_FLOOR,
    TILE_ID_WOODEN_OPEN_DOOR,
    TILE_ID_WOODEN_CLOSED_DOOR,
    TILE_ID_STAIRS_UP,
    TILE_ID_STAIRS_DOWN,
    TILE_ID_UNDEEP_WATER,
    TILE_ID_DEEP_WATER,
    TILE_ID_MUD,
    TILE_ID_MAD_CAP_FUNGUS,
    TILE_ID_MAX,
};

enum tile_types {
    TILE_TYPE_NONE,
    TILE_TYPE_WALL,
    TILE_TYPE_FLOOR,
    TILE_TYPE_DOOR_CLOSED,
    TILE_TYPE_DOOR_OPEN,
    TILE_TYPE_STAIRS_UP,
    TILE_TYPE_STAIRS_DOWN,
    TILE_TYPE_LIQUID,
    TILE_TYPE_MAX,
};

struct tl_tile {
    enum tile_ids id;
    uint16_t attributes;
    enum tile_types type;
    bitfield32_t dungeon_locale;

    char icon;
    int icon_attr;

    uint8_t movement_cost;

    const char *sd_name;
    const char *ld_name;
    const char *plr_enter_str;
    const char *msr_enter_str;
    const char *plr_exit_str;
    const char *msr_exit_str;
};

struct tl_tile *ts_get_tile_specific(enum tile_ids ti);
struct tl_tile *ts_get_tile_type(enum tile_types tt);

void ts_enter(struct tl_tile *tile, struct msr_monster *monster);
void ts_exit(struct tl_tile *tile, struct msr_monster *monster);
void ts_turn_tick_monster(struct tl_tile *tile, struct msr_monster *monster);
void ts_turn_tick(struct tl_tile *tile, coord_t *pos, struct dm_map *map);

#endif /*TILES_H_*/
