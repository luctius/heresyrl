#include <string.h>
#include <ncurses.h>

#include "items.h"
#include "random.h"

static LIST_HEAD(items_list, itm_items_list_entry) items_list_head;
static bool items_list_initialised = false;
static uint64_t id = 1;

#define CLOTHING(item_id,item_sd_name,item_ld_name,cloth_typ,dr,avail,item_quality,item_weight,item_cost,delay) \
    [item_id] = { .id=0, .item_type=ITEM_TYPE_WEARABLE, .availability=avail, .quality=item_quality, \
    .attributes=ITEM_ATTRIBUTE_NONE, .age=0, .weight=item_weight, .cost=item_cost, .sd_name=item_sd_name, \
    .ld_name=item_ld_name, .icon=']', .icon_attr=COLOR_PAIR(DPL_COLOUR_NORMAL), .use_delay=delay, \
    .stacked_quantity=0, .specific.wearable = { .wearable_type=cloth_typ, .damage_reduction=dr, }, }

#define MELEE(item_id,item_sd_name,item_ld_name,wpn_typ,dmg_die,dmg_add,dmg_tp,pen,avail,item_quality,item_weight,item_cost,delay,special) \
    [item_id] = { .id=0, .item_type=ITEM_TYPE_WEAPON, .availability=avail, .quality=item_quality, \
    .attributes=ITEM_ATTRIBUTE_NONE, .age=0, .weight=item_weight, .cost=item_cost, .sd_name=item_sd_name, \
    .ld_name=item_ld_name, .icon='|', .icon_attr=COLOR_PAIR(DPL_COLOUR_NORMAL), .use_delay=delay, \
    .stacked_quantity=0, .specific.weapon = { .weapon_type=wpn_typ, .dmg_type=dmg_tp, .nr_dmg_die=dmg_die, \
    .dmg_addition=dmg_add, .range=0, .rof = { .rof_single=0, .rof_semi=0, .rof_auto=0, }, .magazine_sz=0, \
    .magazine_left=0, .penetration=pen, .special_quality=special, .jammed=false, }, }

#define LIGHT(item_id,item_sd_name,item_ld_name,lumin,dur,avail,item_quality,item_weight,item_cost,delay) \
    [item_id] = { .id=0, .item_type=ITEM_TYPE_TOOL, .availability=avail, .quality=item_quality, \
    .attributes=ITEM_ATTRIBUTE_NONE, .age=0, .weight=item_weight, .cost=item_cost, .sd_name=item_sd_name, \
    .ld_name=item_ld_name, .icon='(', .icon_attr=COLOR_PAIR(DPL_COLOUR_NORMAL), .use_delay=delay, \
    .stacked_quantity=1, .specific.tool = { .tool_type=ITEM_TOOL_TYPE_LIGHT, .energy=dur, .energy_left=dur, \
    .light_luminem=lumin, .lit=false, }, }

#define AMMO(item_id,item_sd_name,item_ld_name,ammo_typ,energ,avail,item_quality,item_weight,item_cost,delay) \
    [item_id] = { .id=0, .item_type=ITEM_TYPE_AMMO, .availability=avail, .quality=item_quality, \
    .attributes=ITEM_ATTRIBUTE_NONE, .age=0, .weight=item_weight, .cost=item_cost, .sd_name=item_sd_name, \
    .ld_name=item_ld_name, .icon='\'', .icon_attr=COLOR_PAIR(DPL_COLOUR_NORMAL), .use_delay=delay, \
    .stacked_quantity=1, .specific.ammo = { .ammo_type=ammo_typ, .energy=energ, energy_left=energ, }, }

struct itm_items static_item_list[] = {
    LIGHT(ITEM_ID_AVERAGE_TORCH,"torch","a torch",3,100,ITEM_AVAILABILITY_PLENTIFUL,ITEM_QUALITY_AVERAGE,1,1,0),
};

void itm_items_list_init(void) {
    if (items_list_initialised == false) {
        items_list_initialised = true;
        LIST_INIT(&items_list_head);
    }
}

void itm_items_list_exit(void) {
    struct itm_items_list_entry *e = NULL;
    while (items_list_head.lh_first != NULL) {
        e = items_list_head.lh_first;
        LIST_REMOVE(items_list_head.lh_first, entries);
        free(e);
    }
}

struct itm_items *itm_get_item_from_list(struct itm_items *prev) {
    if (prev == NULL) return &items_list_head.lh_first->item;
    struct itm_items_list_entry *ile = container_of(prev, struct itm_items_list_entry, item);
    if (ile == NULL) return NULL;
    return &ile->entries.le_next->item;
}

struct itm_items *itm_generate(enum item_types type) {
    if (items_list_initialised == false) itm_items_list_init();
    return NULL;
}

struct itm_items *itm_create_specific(int idx) {
    if (idx >= (int) ARRAY_SZ(static_item_list)) return NULL;

    struct itm_items_list_entry *i = malloc(sizeof(struct itm_items_list_entry) );
    if (i == NULL) return NULL;

    memcpy(&i->item, &static_item_list[idx], sizeof(static_item_list[0]));
    LIST_INSERT_HEAD(&items_list_head, i, entries);
    i->item.id = id++;
    i->item.owner_type = ITEM_OWNER_NONE;

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "Item", "creating: %c", i->item.icon);

    return &i->item;
}

struct itm_items *itm_create_type(enum item_types type, int specific_id) {
    if (items_list_initialised == false) itm_items_list_init();
    return NULL;

}

void itm_destroy(struct itm_items *item) {
    struct itm_items_list_entry *ile = container_of(item, struct itm_items_list_entry, item);

    LIST_REMOVE(ile, entries);
    free(ile);
}

static bool itm_drop_item(struct itm_items *item, struct dc_map *map, int x, int y) {
    bool retval = false;
    if (item == NULL) return false;
    if (map == NULL) return false;
    if (x >= map->x_sz || y >= map->y_sz) return false;

    struct dc_map_entity *target = &SD_GET_INDEX(x, y, map);
    if (TILE_HAS_ATTRIBUTE(target->tile, TILE_ATTR_TRAVERSABLE) ) {
        if (target->item == NULL) {
            target->item = item;
            item->owner_type = ITEM_OWNER_MAP;
            item->owner.owner_map_entity = target;
            retval = true;
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "Item", "You dropped %s.", item->ld_name);
        }
    }

    return retval;
}

bool itm_insert_item(struct itm_items *item, struct dc_map *map, int x, int y) {
    bool retval = false;
    if (item == NULL) return false;
    if (map == NULL) return false;

    if ( (retval = itm_drop_item(item, map, x, y) ) == false) {
        for (int xt = x-1; x < map->x_sz; xt++) {
            for (int yt = y-1; y < map->y_sz; xt++) {
                if (itm_drop_item(item, map, xt, yt) ) {
                    return true;
                }
            }
        }
    }
    return retval;
}

bool itm_remove_item(struct itm_items *item, struct dc_map *map, int x_pos, int y_pos) {
    bool retval = false;
    if (item == NULL) return false;
    if (map == NULL) return false;

    struct dc_map_entity *target = &SD_GET_INDEX(x_pos, y_pos, map);
    if (target->item == item) {
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "Item", "removed (%d,%d)", x_pos, y_pos);
        target->item = NULL;
        item->owner_type = ITEM_OWNER_NONE;
        item->owner.owner_map_entity = NULL;
        retval = true;
    }

    return retval;
}
