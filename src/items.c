#include <string.h>
#include <ncurses.h>

#include "dungeon_creator.h"
#include "items.h"
#include "items_static.h"
#include "random.h"
#include "monster.h"
#include "tiles.h"
#include "inventory.h"

static LIST_HEAD(items_list, itm_item_list_entry) items_list_head;
static bool items_list_initialised = false;
static uint64_t id = 1;

#include "items_static.c"

void itmlst_items_list_init(void) {
    if (items_list_initialised == false) {
        items_list_initialised = true;
        LIST_INIT(&items_list_head);
    }
}

void itmlst_items_list_exit(void) {
    struct itm_item_list_entry *e = NULL;
    while (items_list_head.lh_first != NULL) {
        e = items_list_head.lh_first;
        LIST_REMOVE(items_list_head.lh_first, entries);
        free(e);
    }
        items_list_initialised = false;
}

struct itm_item *itmlst_get_next_item(struct itm_item *prev) {
    if (prev == NULL) {
        if (items_list_head.lh_first != NULL) return &items_list_head.lh_first->item;
        return NULL;
    }
    struct itm_item_list_entry *ile = container_of(prev, struct itm_item_list_entry, item);
    if (ile == NULL) return NULL;
    return &ile->entries.le_next->item;
}

struct itm_item *itm_generate(enum item_types type) {
    if (items_list_initialised == false) itmlst_items_list_init();
    return NULL;
}

struct itm_item *itm_create_specific(int idx) {
    if (idx >= (int) ARRAY_SZ(static_item_list)) return NULL;
    if (items_list_initialised == false) itmlst_items_list_init();

    struct itm_item_list_entry *i = malloc(sizeof(struct itm_item_list_entry) );
    if (i == NULL) return NULL;

    memcpy(&i->item, &static_item_list[idx], sizeof(static_item_list[idx]));
    LIST_INSERT_HEAD(&items_list_head, i, entries);
    i->item.id = id++;
    i->item.owner_type = ITEM_OWNER_NONE;

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "Item", "creating: %c", i->item.icon);

    return &i->item;
}

struct itm_item *itm_create_type(enum item_types type, int specific_id) {
    if (items_list_initialised == false) itmlst_items_list_init();
    return NULL;

}

void itm_destroy(struct itm_item *item) {
    struct itm_item_list_entry *ile = container_of(item, struct itm_item_list_entry, item);

    LIST_REMOVE(ile, entries);
    free(ile);
}

bool itm_insert_item(struct itm_item *item, struct dc_map *map, coord_t *pos) {
    bool retval = false;
    if (item == NULL) return false;
    if (map == NULL) return false;
    if (cd_within_bound(pos, &map->size) == false) return false;

    struct dc_map_entity *target = sd_get_map_me(pos, map);
    if (TILE_HAS_ATTRIBUTE(target->tile, TILE_ATTR_TRAVERSABLE) ) {
        if (inv_has_item(target->inventory, item) == false) {
            if (inv_add_item(target->inventory, item) == true) {
                item->owner_type = ITEM_OWNER_MAP;
                item->owner.owner_map_entity = target;
                retval = true;
                You("dropped %s.", item->ld_name);
            }
        }
    }

    return retval;
}

bool itm_remove_item(struct itm_item *item, struct dc_map *map, coord_t *pos) {
    bool retval = false;
    if (item == NULL) return false;
    if (map == NULL) return false;
    if (cd_within_bound(pos, &map->size) == false) return false;

    struct dc_map_entity *target = sd_get_map_me(pos, map);
    if (inv_has_item(target->inventory, item) == true) {
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "Item", "removed (%d,%d)", pos->x, pos->y);
        if (inv_remove_item(target->inventory, item) ) {
            item->owner_type = ITEM_OWNER_NONE;
            item->owner.owner_map_entity = NULL;
            retval = true;
        }
    }

    return retval;
}

coord_t itm_get_pos(struct itm_item *item) {
    if (item == NULL) return cd_create(0,0);

    switch (item->owner_type) {
        case ITEM_OWNER_MAP:        return item->owner.owner_map_entity->pos;
        case ITEM_OWNER_MONSTER:    return item->owner.owner_monster->pos;
        default:
            return cd_create(0,0);
    }
}

bool wpn_is_type(struct itm_item *item, enum item_weapon_type type) {
    if (item == NULL) return false;
    if (item->item_type != ITEM_TYPE_WEAPON) return false;
    if (item->specific.weapon.weapon_type == type) return true;
    return false;
}

bool wpn_is_catergory(struct itm_item *item, enum item_weapon_category cat) {
    if (item == NULL) return false;
    if (item->item_type != ITEM_TYPE_WEAPON) return false;
    if (item->specific.weapon.weapon_category == cat) return true;
    return false;
}
