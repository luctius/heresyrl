#include "tiles.h"

#include "map_display.h"

#define ARRAY_SZ(a) (sizeof(a) / sizeof(a[0]))

static struct tl_tile tile_array[] = {
    {
        .attributes = 0,
        .type = TILE_TYPE_NONE,
        .colour = DPL_COLOUR_NORMAL,
    },
    {
        .attributes = 0,
        .type = TILE_TYPE_WALL,
        .colour = DPL_COLOUR_NORMAL,
    },
    {
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_OPAGUE,
        .type = TILE_TYPE_FLOOR,
        .colour = DPL_COLOUR_NORMAL,
    },
    {
        .attributes = TILE_ATTR_DOOR_CLOSED,
        .type = TILE_TYPE_DOOR_CLOSED,
        .colour = DPL_COLOUR_NORMAL,
    },
    {
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_OPAGUE | TILE_ATTR_DOOR_OPEN,
        .type = TILE_TYPE_DOOR_OPEN,
        .colour = DPL_COLOUR_NORMAL,
    },
    {
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_OPAGUE | TILE_ATTR_STAIRS_UP,
        .type = TILE_TYPE_STAIRS_UP,
        .colour = DPL_COLOUR_NORMAL,
    },
    {
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_OPAGUE | TILE_ATTR_STAIRS_DOWN,
        .type = TILE_TYPE_STAIRS_DOWN,
        .colour = DPL_COLOUR_NORMAL,
    }
};

struct tl_tile ts_get_tile_type(enum tile_types tt) {
    for (int i = 0; i < ARRAY_SZ(tile_array); i++) {
        if (tile_array[i].type == tt) return tile_array[i];
    }
    return tile_array[0];
}
