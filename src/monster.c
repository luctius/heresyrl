#include <stdlib.h>
#include <stddef.h>
#include <ncurses.h>

#include "dungeon_creator.h"
#include "monster.h"
#include "items.h"

static LIST_HEAD(monster_list, msr_monster_list_entry) head;
struct monster_list *monster_list_head = NULL;
static bool monster_list_initialised = false;

void msr_monster_list_init(void) {
    if (monster_list_initialised == false) {
        monster_list_initialised = true;
        LIST_INIT(&head);
        monster_list_head = &head;
    }
}

void msr_monster_list_exit(void) {
    struct msr_monster_list_entry *e = NULL;
    while (head.lh_first != NULL) {
        e = head.lh_first;
        LIST_REMOVE(head.lh_first, entries);
        free(e);
    }
}

struct msr_monster *msr_create(void) {
    if (monster_list_initialised == false) msr_monster_list_init();

    struct msr_monster_list_entry *m = malloc(sizeof(struct msr_monster_list_entry) );
    if (m != NULL) {
        LIST_INSERT_HEAD(&head, m, entries);

        m->monster.pos = cd_create(0,0);
        m->monster.icon = 'm';
        m->monster.icon_attr = COLOR_PAIR(DPL_COLOUR_NORMAL);
        m->monster.visibility = 100;
        m->monster.is_player = false;
        m->monster.inventory = NULL;

        return &m->monster;
    }
    return NULL;
}

void msr_die(struct msr_monster *monster, struct dc_map *map) {
    if (monster == NULL) return;
    if (map == NULL) return;
    struct msr_monster_list_entry *target_mle = container_of(monster, struct msr_monster_list_entry, monster);

    msr_remove_monster(monster, map);

    LIST_REMOVE(target_mle, entries);
    free(target_mle);
}

bool msr_insert_monster(struct msr_monster *monster, struct dc_map *map, coord_t *pos) {
    bool retval = false;
    if (monster == NULL) return false;
    if (map == NULL) return false;
    if (cd_within_bound(pos, &map->size) == false) return false;

    struct dc_map_entity *me_future = sd_get_map_me(pos, map);
    if (TILE_HAS_ATTRIBUTE(me_future->tile, TILE_ATTR_TRAVERSABLE) ) {
        if (me_future->monster == NULL) {
            me_future->monster = monster;
            monster->pos = *pos;
            retval = true;
        }
    }

    return retval;
}

bool msr_move_monster(struct msr_monster *monster, struct dc_map *map, coord_t *pos) {
    bool retval = false;
    if (monster == NULL) return false;
    if (map == NULL) return false;
    if (cd_within_bound(pos, &map->size) == false) return false;
    if (cd_equal(&monster->pos, pos) == true ) return false;

    struct dc_map_entity *me_current = sd_get_map_me(&monster->pos, map);
    struct dc_map_entity *me_future = sd_get_map_me(pos, map);

    if (TILE_HAS_ATTRIBUTE(me_future->tile, TILE_ATTR_TRAVERSABLE) ) {
        int x_diff = monster->pos.x - pos->x;
        int y_diff = monster->pos.y - pos->y;
        if (abs(x_diff) + abs(y_diff) > 1) return false; /*Speed of one for now*/
        coord_t mon_pos_new = cd_add(&monster->pos, pos);

        if (msr_insert_monster(monster, map, pos) == true) {
            me_current->monster = NULL;
            retval = true;
        }
        else if (msr_move_monster(me_future->monster, map, &mon_pos_new) ) {
            retval = msr_move_monster(monster, map, pos);
        }
    }

    return retval;
}
bool msr_give_item(struct msr_monster *monster, struct itm_items *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (monster->inventory != NULL) {
        Your("inventory is full.");
        return false;
    }

    monster->inventory = item;
    item->owner_type = ITEM_OWNER_MONSTER;
    item->owner.owner_monster = monster;
    You("picked up %s.", item->ld_name);
    return true;
}

bool msr_use_item(struct msr_monster *monster, struct itm_items *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;
    if (monster->inventory == NULL) {
        Your("inventory is empty.");
        return false;
    }

    if (item->item_type == ITEM_TYPE_TOOL && item->specific.tool.tool_type == ITEM_TOOL_TYPE_LIGHT) {
        item->specific.tool.lit = true;
        You("light %s.", item->ld_name);
    }
    return true;
}

bool msr_remove_item(struct msr_monster *monster, struct itm_items *item) {
    if (monster == NULL) return false;
    if (item == NULL) return false;

    monster->inventory = NULL;
    item->owner_type = ITEM_OWNER_NONE;
    item->owner.owner_monster = NULL;
    return true;
}

int msr_get_near_sight_range(struct msr_monster *monster) {
    return 2;
}

int msr_get_far_sight_range(struct msr_monster *monster) {
    return 4;
}

bool msr_remove_monster(struct msr_monster *monster, struct dc_map *map) {
    bool retval = false;
    if (monster == NULL) return false;
    if (map == NULL) return false;

    struct dc_map_entity *me_current = sd_get_map_me(&monster->pos, map);
    if (me_current->monster == monster) {
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "Monster", "removed (%d,%d)", monster->pos.x, monster->pos.y);
        me_current->monster = NULL;
        monster->pos = cd_create(0,0);
        retval = true;
    }

    return retval;
}
