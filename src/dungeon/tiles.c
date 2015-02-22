#include <assert.h>

#include "tiles.h"
#include "heresyrl_def.h"

static struct tl_tile tile_array[] = {
    [TILE_ID_NONE] = {
        .id = TILE_ID_NONE,
        .attributes = TILE_ATTR_BORDER,
        .type = TILE_TYPE_NONE,
        .icon = ' ',
        .icon_attr = TERM_COLOUR_WHITE,
        .movement_cost = 0,
        .sd_name = "",
        .ld_name = "",
    },
    [TILE_ID_TUNNEL_DUMMY] = {
        .id = TILE_ID_TUNNEL_DUMMY,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT,
        .type = TILE_TYPE_FLOOR,
        .icon = 'X',
        .icon_attr = TERM_COLOUR_WHITE,
        .movement_cost = 10,
        .sd_name = "concrete dummy",
        .ld_name = "a concrete dummy",
    },
    [TILE_ID_BORDER_WALL] = {
        .id = TILE_ID_BORDER_WALL,
        .attributes = TILE_ATTR_BORDER,
        .type = TILE_TYPE_WALL,
        .icon = '#',
        .icon_attr = TERM_COLOUR_RED,
        .movement_cost = TILE_MOVEMENT_MAX,
        .sd_name = "concrete wall",
        .ld_name = "a concrete wall",
    },
    [TILE_ID_CONCRETE_WALL] = {
        .id = TILE_ID_CONCRETE_WALL,
        .attributes =  0,
        .type = TILE_TYPE_WALL,
        .icon = '#',
        .icon_attr = TERM_COLOUR_UMBER,
        .movement_cost = 30,
        .sd_name = "concrete wall",
        .ld_name = "a concrete wall",
    },
    [TILE_ID_CONCRETE_WALL_LIT] = {
        .id = TILE_ID_CONCRETE_WALL_LIT,
        .attributes =  TILE_ATTR_LIGHT_SOURCE,
        .type = TILE_TYPE_WALL,
        .icon = '#',
        .icon_attr = TERM_COLOUR_UMBER,
        .movement_cost = 30,
        .sd_name = "lit concrete wall",
        .ld_name = "a concrete wall with a fixed light",
    },
    [TILE_ID_CONCRETE_FLOOR] = {
        .id = TILE_ID_CONCRETE_FLOOR,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT,
        .type = TILE_TYPE_FLOOR,
        .icon = '.',
        .icon_attr = TERM_COLOUR_WHITE,
        .movement_cost = 10,
        .sd_name = "concrete floor",
        .ld_name = "a concrete floor",
    },
    [TILE_ID_WOODEN_CLOSED_DOOR] = {
        .id = TILE_ID_WOODEN_CLOSED_DOOR,
        .attributes = TILE_ATTR_DOOR_CLOSED,
        .type = TILE_TYPE_DOOR_CLOSED,
        .icon = '+',
        .icon_attr = TERM_COLOUR_UMBER,
        .movement_cost = 20,
        .sd_name = "wooden door",
        .ld_name = "a wooden door",
    },
    [TILE_ID_WOODEN_OPEN_DOOR] = {
        .id = TILE_ID_WOODEN_OPEN_DOOR,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT | TILE_ATTR_DOOR_OPEN,
        .type = TILE_TYPE_DOOR_OPEN,
        .icon = '.',
        .icon_attr = TERM_COLOUR_UMBER,
        .movement_cost = 10,
        .sd_name = "wooden door",
        .ld_name = "a wooden door",
    },
    [TILE_ID_STAIRS_UP] = {
        .id = TILE_ID_STAIRS_UP,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT | TILE_ATTR_STAIRS_UP,
        .type = TILE_TYPE_STAIRS_UP,
        .icon = '<',
        .icon_attr = TERM_COLOUR_WHITE,
        .movement_cost = 10,
        .sd_name = "concrete stairs",
        .ld_name = "concrete stairs, going up",
    },
    [TILE_ID_STAIRS_DOWN] = {
        .id = TILE_ID_STAIRS_DOWN,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT | TILE_ATTR_STAIRS_DOWN,
        .type = TILE_TYPE_STAIRS_DOWN,
        .icon = '>',
        .icon_attr = TERM_COLOUR_WHITE,
        .movement_cost = 10,
        .sd_name = "concrete stairs",
        .ld_name = "concrete stairs, going down",
    },
    [TILE_ID_UNDEEP_WATER] = {
        .id = TILE_ID_UNDEEP_WATER,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT,
        .type = TILE_TYPE_LIQUID,
        .icon = '~',
        .icon_attr = TERM_COLOUR_BLUE,
        .movement_cost = 20,
        .sd_name = "undeep water",
        .ld_name = "undeep water",
        .plr_enter_str = "start to wade into undeep water.",
        .msr_enter_str = "starts to wade into undeep water.",
        .plr_exit_str = "step out of the water.",
        .msr_exit_str = "steps out of the water.",
    },
    [TILE_ID_DEEP_WATER] = {
        .id = TILE_ID_DEEP_WATER,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT,
        .type = TILE_TYPE_LIQUID,
        .icon = '~',
        .icon_attr = TERM_COLOUR_DEEP_L_BLUE,
        .movement_cost = 30,
        .sd_name = "deep water",
        .ld_name = "deep water",
        .plr_enter_str = "start swimming.",
        .msr_enter_str = "starts swimming.",
        .plr_exit_str = "stop swimming.",
        .msr_exit_str = "stops swimming.",
    },
    [TILE_ID_MUD] = {
        .id = TILE_ID_MUD,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT,
        .type = TILE_TYPE_LIQUID,
        .icon = '~',
        .icon_attr = TERM_COLOUR_UMBER,
        .movement_cost = 15,
        .sd_name = "mud",
        .ld_name = "a muddy area",
        .plr_enter_str = "step into the mud.",
        .msr_enter_str = "steps into the mud.",
        .plr_exit_str = "step out of the mud.",
        .msr_exit_str = "steps out of the mud.",
    },
};

static bool processed = false;
static void process_tiles(void) {
    if (processed) return;

    for (unsigned int i = 0; i < ARRAY_SZ(tile_array); i++) {
        tile_array[i].icon_attr = get_colour(tile_array[i].icon_attr);
    }

    processed = true;
}

struct tl_tile *ts_get_tile_specific(enum tile_ids ti) {
    assert(ti < TILE_ID_MAX);

    process_tiles();
    return &tile_array[ti];
}

struct tl_tile *ts_get_tile_type(enum tile_types tt) {
    process_tiles();
    for (unsigned int i = 0; i < ARRAY_SZ(tile_array); i++) {
        if (tile_array[i].type == tt) return &tile_array[i];
    }
    return &tile_array[0];
}

void ts_enter(struct tl_tile *tile, struct msr_monster *monster) {
    switch (tile->id) {
        case TILE_ID_NONE: break;
        case TILE_ID_MAX: break;
        case TILE_ID_TUNNEL_DUMMY: break;
        case TILE_ID_BORDER_WALL: break;
        case TILE_ID_CONCRETE_WALL: break;
        case TILE_ID_CONCRETE_WALL_LIT: break;
        case TILE_ID_CONCRETE_FLOOR: break;
        case TILE_ID_WOODEN_OPEN_DOOR: break;
        case TILE_ID_WOODEN_CLOSED_DOOR: break;
        case TILE_ID_STAIRS_UP: break;
        case TILE_ID_STAIRS_DOWN: break;
        case TILE_ID_UNDEEP_WATER:
            se_add_status_effect(monster, SEID_WADE, tile->sd_name);
            break;
        case TILE_ID_DEEP_WATER:
            se_add_status_effect(monster, SEID_SWIMMING, tile->sd_name);
            break;
        case TILE_ID_MUD:
            se_add_status_effect(monster, SEID_MUD, tile->sd_name);
            break;
    }
}

void ts_exit(struct tl_tile *tile, struct msr_monster *monster) {
    switch (tile->id) {
        case TILE_ID_NONE: break;
        case TILE_ID_MAX: break;
        case TILE_ID_TUNNEL_DUMMY: break;
        case TILE_ID_BORDER_WALL: break;
        case TILE_ID_CONCRETE_WALL: break;
        case TILE_ID_CONCRETE_WALL_LIT: break;
        case TILE_ID_CONCRETE_FLOOR: break;
        case TILE_ID_WOODEN_OPEN_DOOR: break;
        case TILE_ID_WOODEN_CLOSED_DOOR: break;
        case TILE_ID_STAIRS_UP: break;
        case TILE_ID_STAIRS_DOWN: break;
        case TILE_ID_UNDEEP_WATER:
            se_remove_effects_by_tid(monster, SEID_WADE);
            break;
        case TILE_ID_DEEP_WATER:
            se_remove_effects_by_tid(monster, SEID_SWIMMING);
            break;
        case TILE_ID_MUD:
            se_remove_effects_by_tid(monster, SEID_MUD);
            break;
    }
}

void ts_turn_tick(struct tl_tile *tile, struct msr_monster *monster) {
    switch (tile->id) {
        case TILE_ID_NONE: break;
        case TILE_ID_MAX: break;
        case TILE_ID_TUNNEL_DUMMY: break;
        case TILE_ID_BORDER_WALL: break;
        case TILE_ID_CONCRETE_WALL: break;
        case TILE_ID_CONCRETE_WALL_LIT: break;
        case TILE_ID_CONCRETE_FLOOR: break;
        case TILE_ID_WOODEN_OPEN_DOOR: break;
        case TILE_ID_WOODEN_CLOSED_DOOR: break;
        case TILE_ID_STAIRS_UP: break;
        case TILE_ID_STAIRS_DOWN: break;
        case TILE_ID_UNDEEP_WATER: break;
        case TILE_ID_DEEP_WATER: break;
        case TILE_ID_MUD: break;
    }
}

