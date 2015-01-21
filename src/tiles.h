#ifndef TILES_H_
#define TILES_H_

#include <stdint.h>

#include "monster/status_effects.h"

#define TILE_HAS_ATTRIBUTE(tile, attr) ((tile->attributes & attr) > 0)
#define TILE_MOVEMENT_MAX (UINT8_MAX)

enum tile_attributes {
    TILE_ATTR_TRAVERSABLE    = (1<<0),
    TILE_ATTR_OPAGUE         = (1<<1),
    TILE_ATTR_DOOR_CLOSED    = (1<<2),
    TILE_ATTR_DOOR_OPEN      = (1<<3),
    TILE_ATTR_STAIRS_UP      = (1<<4),
    TILE_ATTR_STAIRS_DOWN    = (1<<5),
    TILE_ATTR_HAZARDOUS      = (1<<6),
    TILE_ATTR_BORDER         = (1<<7),
    TILE_ATTR_LIGHT_SOURCE   = (1<<8),
    TILE_ATTR_MAX,
};

enum tile_ids {
    TILE_ID_NONE,
    TILE_ID_BORDER_WALL,
    TILE_ID_CONCRETE_WALL,
    TILE_ID_CONCRETE_WALL_LIT,
    TILE_ID_CONCRETE_FLOOR,
    TILE_ID_WOODEN_OPEN_DOOR,
    TILE_ID_WOODEN_CLOSED_DOOR,
    TILE_ID_CONCRETE_STAIRS_UP,
    TILE_ID_CONCRETE_STAIRS_DOWN,
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
    TILE_TYPE_MAX,
};

struct tl_tile {
    enum tile_ids id;
    uint16_t attributes;
    enum tile_types type;

    char icon;
    int icon_attr;

    uint8_t movement_cost;
    enum se_ids status_effect_id;

    const char *sd_name;
    const char *ld_name;
};

struct tl_tile *ts_get_tile_specific(enum tile_ids ti);
struct tl_tile *ts_get_tile_type(enum tile_types tt);

#endif /*TILES_H_*/
