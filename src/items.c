#include <string.h>
#include <ncurses.h>

#include "dungeon_creator.h"
#include "items.h"
#include "items_static.h"
#include "random.h"
#include "monster.h"
#include "tiles.h"
#include "inventory.h"

struct itm_item_list_entry {
    struct itm_item item;
    LIST_ENTRY(itm_item_list_entry) entries;
};

static LIST_HEAD(items_list, itm_item_list_entry) items_list_head;
static bool items_list_initialised = false;
static uint64_t uid = 1;

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

struct itm_item *itmlst_item_by_uid(uint32_t item_uid) {
    if (items_list_initialised == false) return false;
    struct itm_item_list_entry *ie = items_list_head.lh_first;

    while (ie != NULL) {
        if (item_uid == ie->item.uid) return &ie->item;
        ie = ie->entries.le_next;
    }
    return NULL;
}

static uint32_t itmlst_next_id(void) {
    if (items_list_initialised == false) return false;
    struct itm_item_list_entry *ie = items_list_head.lh_first;
    uid = 1;

    while (ie != NULL) {
        if (uid <= ie->item.uid) uid = ie->item.uid+1;
        ie = ie->entries.le_next;
    }
    return uid;
}

struct itm_item *itm_generate(enum item_types type) {
    if (items_list_initialised == false) itmlst_items_list_init();
    return NULL;
}

struct itm_item *itm_create_specific(int template_id) {
    if (template_id >= (int) ARRAY_SZ(static_item_list)) return NULL;
    if (items_list_initialised == false) itmlst_items_list_init();

    struct itm_item_list_entry *i = malloc(sizeof(struct itm_item_list_entry) );
    if (i == NULL) return NULL;

    memcpy(&i->item, &static_item_list[template_id], sizeof(static_item_list[template_id]));
    LIST_INSERT_HEAD(&items_list_head, i, entries);
    i->item.uid = itmlst_next_id();
    i->item.owner_type = ITEM_OWNER_NONE;

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "Item", "creating: %c", i->item.icon);

    return &i->item;
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

bool wpn_ranged_weapon_setting_check(struct itm_item *item, enum wpn_rof_setting set) {
    if (wpn_is_type(item, WEAPON_TYPE_RANGED) == false) return false;
    if (item->specific.weapon.rof[set] > 0) return true;
    return false;
}

