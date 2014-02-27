#include <string.h>

#include "items.h"
#include "random.h"

static LIST_HEAD(items_list, itm_items_list_entry) head;
struct items_list *items_list_head = NULL;
static bool items_list_initialised = false;
static uint64_t id = 1;

#define CLOTHING(nme,cloth_typ,dr,avail,wght,cst,delay) \
    { .id=0, .item_type=ITEM_TYPE_WEARABLE, .availability=avail, .quality=ITEM_QUALITY_AVERAGE, \
    .attributes=ITEM_ATTRIBUTE_NONE, .age=0, .weight=wght, .cost=cst, .name=nme, .icon=']', .colour=DPL_COLOUR_NORMAL, \
    .use_delay=delay, .stacked_quantity=0, .specific.wearable = { .wearable_type=cloth_typ, .damage_reduction=dr, }, }

#define MELEE(nme,wpn_typ,dmg_die,dmg_add,dmg_tp,pen,avail,wght,cst,delay,special) \
    { .id=0, .item_type=ITEM_TYPE_WEAPON, .availability=avail, .quality=ITEM_QUALITY_AVERAGE, \
    .attributes=ITEM_ATTRIBUTE_NONE, .age=0, .weight=wght, .cost=cst, .name=nme, .icon='|', .colour=DPL_COLOUR_NORMAL, \
    .use_delay=delay, .stacked_quantity=0, .specific.weapon = { .weapon_type=wpn_typ, .dmg_type=dmg_tp, \
    .nr_dmg_die=dmg_die, .dmg_addition=dmg_add, .range=0, .rof = { .rof_single=0, .rof_semi=0, .rof_auto=0, }, \
    .penetration=pen, .special_quality=special, .jammed=false, }, }

#define LIGHT(nme,lumin,dur,avail,wght,cst,delay) \
    { .id=0, .item_type=ITEM_TYPE_TOOL, .availability=avail, .quality=ITEM_QUALITY_AVERAGE, \
    .attributes=ITEM_ATTRIBUTE_NONE, .age=0, .weight=wght, .cost=cst, .name=nme, .icon='(', .colour=DPL_COLOUR_NORMAL, \
    .use_delay=delay, .stacked_quantity=1, .specific.tool = { .tool_type=ITEM_TOOL_TYPE_LIGHT, \
    .energy=dur, .energy_left=dur, .light_luminem=lumin, .lit=false, }, }

#define AMMO(nme,ammo_typ,energ,avail,wght,cst,delay) \
    { .id=0, .item_type=ITEM_TYPE_AMMO, .availability=avail, .quality=ITEM_QUALITY_AVERAGE, \
    .attributes=ITEM_ATTRIBUTE_NONE, .age=0, .weight=wght, .cost=cst, .name=nme, .icon='\'', .colour=DPL_COLOUR_NORMAL, \
    .use_delay=delay, .stacked_quantity=1, .specific.ammo = { .ammo_type=ammo_typ, \
    .energy=energ, energy_left=energ, }, }

struct itm_items static_item_list[] = {
    LIGHT("torch",10,100,ITEM_AVAILABILITY_PLENTIFUL,1,1,0),

};

void itm_items_list_init(void) {
    if (items_list_initialised == false) {
        items_list_initialised = true;
        LIST_INIT(&head);
        items_list_head = &head;
    }
}

void itm_items_list_exit(void) {
    struct itm_items_list_entry *e = NULL;
    while (head.lh_first != NULL) {
        e = head.lh_first;
        LIST_REMOVE(head.lh_first, entries);
        free(e);
    }
}

struct itm_items *itm_generate(enum item_types type) {
    if (items_list_initialised == false) itm_items_list_init();
    return NULL;
}

struct itm_items *itm_create_specific(int idx) {
    if (idx >= (int) ARRAY_SZ(static_item_list)) return NULL;

    struct itm_items_list_entry *i = malloc(sizeof(struct itm_items_list_entry) );
    if (i == NULL) return NULL;

    memcpy(i, &static_item_list[idx], sizeof(static_item_list[0]));
    LIST_INSERT_HEAD(&head, i, entries);
    i->item.id = id++;

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
    if (x >= map->x_sz || y >= map->y_sz) return false;

    struct dc_map_entity *target = &SD_GET_INDEX(x, y, map);
    if (TILE_HAS_ATTRIBUTE(target->tile, TILE_ATTR_TRAVERSABLE) ) {
        if (target->item == NULL) {
            target->item = item;
            retval = true;
            lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "Item", "insert (%d,%d)", x, y);
        }
    }

    return retval;
}

bool itm_insert_item(struct itm_items *item, struct dc_map *map, int x, int y) {
    bool retval = false;
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

    struct dc_map_entity *target = &SD_GET_INDEX(x_pos, y_pos, map);
    if (target->item == item) {
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "Item", "removed (%d,%d)", x_pos, y_pos);
        target->item = NULL;
        retval = true;
    }

    return retval;
}
