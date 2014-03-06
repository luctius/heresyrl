#include <ncurses.h>
#include "tiles.h"
#include "heresyrl_def.h"

static struct tl_tile tile_array[] = {
    [TILE_ID_NONE] = {
        .attributes = TILE_ATTR_BORDER,
        .type = TILE_TYPE_NONE,
        .icon = ' ',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = 0,
        .sd_name = "",
        .ld_name = "",
    },
    [TILE_ID_BORDER_WALL] = {
        .attributes = TILE_ATTR_BORDER,
        .type = TILE_TYPE_WALL,
        .icon = '#',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = TILE_MOVEMENT_MAX,
        .sd_name = "stone wall",
        .ld_name = "a stone wall",
    },
    [TILE_ID_STONE_WALL] = {
        .attributes =  0,
        .type = TILE_TYPE_WALL,
        .icon = '#',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = 3,
        .sd_name = "stone wall",
        .ld_name = "a stone wall",
    },
    [TILE_ID_STONE_FLOOR] = {
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_OPAGUE,
        .type = TILE_TYPE_FLOOR,
        .icon = '.',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = 1,
        .sd_name = "stone floor",
        .ld_name = "a stone floor",
    },
    [TILE_ID_WOODEN_CLOSED_DOOR] = {
        .attributes = TILE_ATTR_DOOR_CLOSED,
        .type = TILE_TYPE_DOOR_CLOSED,
        .icon = '+',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = 2,
        .sd_name = "wooden door",
        .ld_name = "a wooden door",
    },
    [TILE_ID_WOODEN_OPEN_DOOR] = {
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_OPAGUE | TILE_ATTR_DOOR_OPEN,
        .type = TILE_TYPE_DOOR_OPEN,
        .icon = '/',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = 1,
        .sd_name = "wooden door",
        .ld_name = "a wooden door",
    },
    [TILE_ID_STONE_STAIRS_UP] = {
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_OPAGUE | TILE_ATTR_STAIRS_UP,
        .type = TILE_TYPE_STAIRS_UP,
        .icon = '<',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = 1,
        .sd_name = "stone stairs",
        .ld_name = "stone stairs, going up",
    },
    [TILE_ID_STONE_STAIRS_DOWN] = {
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_OPAGUE | TILE_ATTR_STAIRS_DOWN,
        .type = TILE_TYPE_STAIRS_DOWN,
        .icon = '>',
        .icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL),
        .movement_cost = 1,
        .sd_name = "stone stairs",
        .ld_name = "stone stairs, going down",
    },
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

int ts_get_movement_cost_max(void) {
    int max = 0;
    for (unsigned int i = 0; i < ARRAY_SZ(tile_array); i++) {
        if (max < tile_array[i].movement_cost) max = tile_array[i].movement_cost;
    }
    return max;
}
