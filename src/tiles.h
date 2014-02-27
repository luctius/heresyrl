#pragma once
#ifndef TILES_H_
#define TILES_H_

#include <stdint.h>
#include "heresyrl_def.h"

#define TILE_HAS_ATTRIBUTE(tile, attr) ((tile.attributes & attr) > 0)
#define TILE_ATTR_TRAVERSABLE (1<<0)
#define TILE_ATTR_OPAGUE (1<<1)
#define TILE_ATTR_DOOR_CLOSED (1<<2)
#define TILE_ATTR_DOOR_OPEN (1<<3)
#define TILE_ATTR_STAIRS_UP (1<<4)
#define TILE_ATTR_STAIRS_DOWN (1<<5)
#define TILE_ATTR_HAZARDOUS (1<<6)

struct tl_tile {
    uint8_t attributes;
    uint8_t type;
    uint8_t colour;
};

enum tile_types {
    TILE_TYPE_NONE          = ' ',
    TILE_TYPE_WALL          = '#',
    TILE_TYPE_FLOOR         = '.',
    TILE_TYPE_DOOR_CLOSED   = '+',
    TILE_TYPE_DOOR_OPEN     = '/',
    TILE_TYPE_STAIRS_UP     = '<',
    TILE_TYPE_STAIRS_DOWN   = '>',
};

struct tl_tile ts_get_tile_type(enum tile_types tt);

#endif /*TILES_H_*/
