#ifndef TILES_H_
#define TILES_H_

#include <stdint.h>

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
    TILE_ATTR_MAX,
};

enum tile_ids {
    TILE_ID_NONE,
    TILE_ID_BORDER_WALL,
    TILE_ID_STONE_WALL,
    TILE_ID_STONE_FLOOR,
    TILE_ID_WOODEN_OPEN_DOOR,
    TILE_ID_WOODEN_CLOSED_DOOR,
    TILE_ID_STONE_STAIRS_UP,
    TILE_ID_STONE_STAIRS_DOWN,
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
    uint16_t attributes;
    enum tile_types type;
    char icon;
    int icon_attr;
    uint8_t movement_cost;
    const char *sd_name;
    const char *ld_name;
    const char *description;
};

struct tl_tile *ts_get_tile_specific(enum tile_ids ti);
struct tl_tile *ts_get_tile_type(enum tile_types tt);
int ts_get_movement_cost_max(void);

#endif /*TILES_H_*/