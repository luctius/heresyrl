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

#include <assert.h>

#include "tiles.h"
#include "heresyrl_def.h"
#include "random.h"
#include "coord.h"

static struct tl_tile tile_array[] = {
    [TILE_ID_NONE] = {
        .id = TILE_ID_NONE,
        .attributes = TILE_ATTR_BORDER,
        .type = TILE_TYPE_NONE,
        .icon = L' ',
        .icon_attr = TERM_COLOUR_WHITE,
        .movement_cost = 0,
        .sd_name = L"",
        .ld_name = L"",
        .replacement = TILE_ID_NONE,
    },
    [TILE_ID_DUMMY] = {
        .id = TILE_ID_DUMMY,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT,
        .type = TILE_TYPE_FLOOR,
        .icon = L'X',
        .icon_attr = TERM_COLOUR_WHITE,
        .movement_cost = 10,
        .sd_name = L"concrete dummy",
        .ld_name = L"a concrete dummy",
        .replacement = TILE_ID_NONE,
    },
    [TILE_ID_DUMMY_2] = {
        .id = TILE_ID_DUMMY_2,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT,
        .type = TILE_TYPE_FLOOR,
        .icon = L'+',
        .icon_attr = TERM_COLOUR_WHITE,
        .movement_cost = 10,
        .sd_name = L"concrete dummy",
        .ld_name = L"a concrete dummy",
        .replacement = TILE_ID_NONE,
    },
    [TILE_ID_BORDER_WALL] = {
        .id = TILE_ID_BORDER_WALL,
        .attributes = TILE_ATTR_BORDER,
        .type = TILE_TYPE_WALL,
        .icon = L'#',
        .icon_attr = TERM_COLOUR_RED,
        .movement_cost = TILE_MOVEMENT_MAX,
        .sd_name = L"concrete wall",
        .ld_name = L"a concrete wall",
        .replacement = TILE_ID_NONE,
    },
    [TILE_ID_CONCRETE_WALL] = {
        .id = TILE_ID_CONCRETE_WALL,
        .attributes =  0,
        .type = TILE_TYPE_WALL,
        .icon = L'█', //L'#', 
        .icon_attr = TERM_COLOUR_UMBER,
        .movement_cost = 30,
        .sd_name = L"concrete wall",
        .ld_name = L"a concrete wall",
        .replacement = TILE_ID_NONE,
    },
    [TILE_ID_BRASSIER] = {
        .id = TILE_ID_BRASSIER,
        .attributes =  TILE_ATTR_LIGHT_SOURCE | TILE_ATTR_TRANSPARENT,
        .type = TILE_TYPE_WALL,
        .icon = L'Y',
        .icon_attr = TERM_COLOUR_YELLOW,
        .movement_cost = 30,
        .sd_name = L"brassier",
        .ld_name = L"a lit brassier",
        .replacement = TILE_ID_NONE,
    },
    [TILE_ID_CONCRETE_FLOOR] = {
        .id = TILE_ID_CONCRETE_FLOOR,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT,
        .type = TILE_TYPE_FLOOR,
        .icon = L'.',
        .icon_attr = TERM_COLOUR_WHITE,
        .movement_cost = 10,
        .sd_name = L"concrete floor",
        .ld_name = L"a concrete floor",
        .replacement = TILE_ID_NONE,
    },
    [TILE_ID_WOODEN_CLOSED_DOOR] = {
        .id = TILE_ID_WOODEN_CLOSED_DOOR,
        .attributes = TILE_ATTR_DOOR_CLOSED,
        .type = TILE_TYPE_DOOR_CLOSED,
        .icon = L'+',
        .icon_attr = TERM_COLOUR_UMBER,
        .movement_cost = 20,
        .sd_name = L"wooden door",
        .ld_name = L"a wooden door",
        .replacement = TILE_ID_WOODEN_OPEN_DOOR,
    },
    [TILE_ID_WOODEN_OPEN_DOOR] = {
        .id = TILE_ID_WOODEN_OPEN_DOOR,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT | TILE_ATTR_DOOR_OPEN,
        .type = TILE_TYPE_DOOR_OPEN,
        .icon = L'\'',
        .icon_attr = TERM_COLOUR_UMBER,
        .movement_cost = 10,
        .sd_name = L"wooden door",
        .ld_name = L"a wooden door",
        .replacement = TILE_ID_WOODEN_CLOSED_DOOR,
    },
    [TILE_ID_STAIRS_UP] = {
        .id = TILE_ID_STAIRS_UP,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT | TILE_ATTR_STAIRS_UP,
        .type = TILE_TYPE_STAIRS_UP,
        .icon = L'<',
        .icon_attr = TERM_COLOUR_WHITE,
        .movement_cost = 10,
        .sd_name = L"concrete stairs",
        .ld_name = L"concrete stairs, going up",
        .replacement = TILE_ID_NONE,
    },
    [TILE_ID_STAIRS_DOWN] = {
        .id = TILE_ID_STAIRS_DOWN,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT | TILE_ATTR_STAIRS_DOWN,
        .type = TILE_TYPE_STAIRS_DOWN,
        .icon = L'>',
        .icon_attr = TERM_COLOUR_WHITE,
        .movement_cost = 10,
        .sd_name = L"concrete stairs",
        .ld_name = L"concrete stairs, going down",
        .replacement = TILE_ID_NONE,
    },
    [TILE_ID_UNDEEP_WATER] = {
        .id = TILE_ID_UNDEEP_WATER,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT,
        .type = TILE_TYPE_LIQUID,
        .icon = L'~',
        .icon_attr = TERM_COLOUR_BLUE,
        .movement_cost = 20,
        .sd_name = L"undeep water",
        .ld_name = L"undeep water",
        .plr_enter_str = L"start to wade into undeep water.",
        .msr_enter_str = L"starts to wade into undeep water.",
        .plr_exit_str = L"step out of the water.",
        .msr_exit_str = L"steps out of the water.",
        .replacement = TILE_ID_NONE,
    },
    [TILE_ID_DEEP_WATER] = {
        .id = TILE_ID_DEEP_WATER,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT,
        .type = TILE_TYPE_LIQUID,
        .icon = L'~',
        .icon_attr = TERM_COLOUR_DEEP_L_BLUE,
        .movement_cost = 30,
        .sd_name = L"deep water",
        .ld_name = L"deep water",
        .plr_enter_str = L"start swimming.",
        .msr_enter_str = L"starts swimming.",
        .plr_exit_str = L"stop swimming.",
        .msr_exit_str = L"stops swimming.",
        .replacement = TILE_ID_NONE,
    },
    [TILE_ID_MUD] = {
        .id = TILE_ID_MUD,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT,
        .type = TILE_TYPE_LIQUID,
        .icon = L'~',
        .icon_attr = TERM_COLOUR_UMBER,
        .movement_cost = 15,
        .sd_name = L"mud",
        .ld_name = L"a muddy area",
        .plr_enter_str = L"step into the mud.",
        .msr_enter_str = L"steps into the mud.",
        .plr_exit_str = L"step out of the mud.",
        .msr_exit_str = L"steps out of the mud.",
        .replacement = TILE_ID_NONE,
    },

    [TILE_ID_MAD_CAP_FUNGUS] = {
        .id = TILE_ID_MAD_CAP_FUNGUS,
        .attributes = TILE_ATTR_TRAVERSABLE | TILE_ATTR_TRANSPARENT,
        .type = TILE_TYPE_FLOOR,
        .icon = L'.',
        .icon_attr = TERM_COLOUR_GREEN,
        .movement_cost = 10,
        .sd_name = L"mushrooms",
        .ld_name = L"a growth of mushroom",
        .replacement = TILE_ID_NONE,
    },
};

struct tl_tile *ts_get_tile_specific(enum tile_ids ti) {
    assert(ti < TILE_ID_MAX);

    return &tile_array[ti];
}

struct tl_tile *ts_get_tile_type(enum tile_types tt) {
    for (unsigned int i = 0; i < ARRAY_SZ(tile_array); i++) {
        if (tile_array[i].type == tt) return &tile_array[i];
    }
    return &tile_array[0];
}

void ts_enter(struct tl_tile *tile, struct msr_monster *monster) {
    switch (tile->id) {
        default: break;
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
        default: break;
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

void ts_turn_tick_monster(struct tl_tile *tile, struct msr_monster *monster) {
    FIX_UNUSED(monster);
    switch (tile->id) {
        default: break;
    }
}

void ts_turn_tick(struct tl_tile *tile, coord_t *pos, struct dm_map *map) {
    switch (tile->id) {
        default: break;
        case TILE_ID_MAD_CAP_FUNGUS:  {
            int r = random_xd10(gbl_game->random, 1);
            coord_t p = { .x = 0, .y = 0, };
            if (r == 1) {
                p = *pos;
            }
            else if (r == 2) {
                int pos_mod = random_int32(gbl_game->random) % coord_nhlo_table_sz;
                p = cd_add(&coord_nhlo_table[pos_mod], pos);
            }

            if (p.x != 0 && p.y != 0) {
                struct dm_map_entity *me = dm_get_map_me(&p, map);
                if ( (random_int32(gbl_game->random)  % 1000) == 30) {
                    ge_create(GEID_MAD_CAP_CLOUD, me);
                }
                if (inv_inventory_size(me->inventory) == 0) {
                    if ( (random_int32(gbl_game->random)  % 1000) == 2) {
                        struct itm_item *item = itm_create(IID_MUSHROOM_MAD_CAP);
                        assert(itm_insert_item(item, map, &p) );
                    }
                }
            }
        } break;
    }
}

