#include <ncurses.h>
#include "tiles.h"
#include "map_display.h"

static struct tl_tile tile_array[] = {
    [TILE_ID_NONE] = {
        .attributes = 0,
        .type = TILE_TYPE_NONE,
        .icon = ' ',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = 0,
    },
    [TILE_ID_BORDER_WALL] = {
        .attributes = TILE_ATTR_BORDER,
        .type = TILE_TYPE_WALL,
        .icon = '#',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = TILE_MOVEMENT_MAX,
    },
    [TILE_ID_STONE_WALL] = {
        .attributes = 0,
        .type = TILE_TYPE_WALL,
        .icon = '#',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = 3,
    },
    [TILE_ID_STONE_FLOOR] = {
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_OPAGUE,
        .type = TILE_TYPE_FLOOR,
        .icon = '.',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = 1,
    },
    [TILE_ID_WOODEN_CLOSED_DOOR] = {
        .attributes = TILE_ATTR_DOOR_CLOSED,
        .type = TILE_TYPE_DOOR_CLOSED,
        .icon = '+',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = 2,
    },
    [TILE_ID_WOODEN_OPEN_DOOR] = {
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_OPAGUE | TILE_ATTR_DOOR_OPEN,
        .type = TILE_TYPE_DOOR_OPEN,
        .icon = '/',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = 1,
    },
    [TILE_ID_STONE_STAIRS_UP] = {
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_OPAGUE | TILE_ATTR_STAIRS_UP,
        .type = TILE_TYPE_STAIRS_UP,
        .icon = '<',
        //.icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .icon_attr = COLOR_PAIR(DPL_COLOUR_BGB_CYAN),
        .movement_cost = 1,
    },
    [TILE_ID_STONE_STAIRS_DOWN] = {
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_OPAGUE | TILE_ATTR_STAIRS_DOWN,
        .type = TILE_TYPE_STAIRS_DOWN,
        .icon = '>',
        //.icon_attr = DPL_COLOUR_NORMAL,
        .icon_attr = DPL_COLOUR_BGB_CYAN,
        .movement_cost = 1,
    }
};

struct tl_tile *ts_get_tile_specific(enum tile_ids ti) {
    if (ti >= TILE_ID_MAX) return 0;
    return &tile_array[ti];
}

struct tl_tile *ts_get_tile_type(enum tile_types tt) {
    for (unsigned int i = 0; i < ARRAY_SZ(tile_array); i++) {
        if (tile_array[i].type == tt) return &tile_array[i];
    }
    return &tile_array[0];
}
