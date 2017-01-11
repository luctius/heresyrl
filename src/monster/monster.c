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

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/param.h>
#include <assert.h>

#include "monster.h"
#include "monster_static.h"
#include "status_effects/status_effects.h"
#include "game.h"
#include "random.h"
#include "turn_tick.h"
#include "inventory.h"
#include "items/items.h"
#include "items/items_static.h"
#include "dungeon/tiles.h"
#include "dungeon/dungeon_map.h"
#include "dowear.h"
#include "random_generator.h"
#include "fov/sight.h"

static TAILQ_HEAD(monster_list, msr_monster_list_entry) monster_list_head;
static bool monster_list_initialised = false;

#include "monster_static_def.h"

struct msr_monster_list_entry {
    struct msr_monster monster;
    TAILQ_ENTRY(msr_monster_list_entry) entries;
};

void msrlst_monster_list_init(void) {
    for (unsigned int i = 1; i < ARRAY_SZ(static_monster_list); i++) {
        struct msr_monster *template_monster = &static_monster_list[i];
        if ( (template_monster->icon == '\0') || (template_monster->sd_name == NULL) || (template_monster->ld_name == NULL) ){
            fprintf(stderr, "Monster list integrity check failed! [%d]\n", i);
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < MSR_TALENT_TIER_MAX; j++) {
            if ( ( (template_monster->talents[j] & MSR_TALENT_HEADER_MASK) != 0) &&
                ( ( (template_monster->talents[j] & MSR_TALENT_HEADER_MASK) != MSR_TALENT_HEADER(j) ) ) ) {
                fprintf(stderr, "Monster list integrity check failed on talents! [%d:%d]\n", i, j);
                exit(EXIT_FAILURE);
            }
        }
    }

    if (monster_list_initialised == false) {
        monster_list_initialised = true;
        TAILQ_INIT(&monster_list_head);
    }
}

void msrlst_monster_list_exit(void) {
    struct msr_monster_list_entry *e = NULL;
    while (monster_list_head.tqh_first != NULL) {
        e = monster_list_head.tqh_first;
        TAILQ_REMOVE(&monster_list_head, monster_list_head.tqh_first, entries);
        msr_clear_controller(&e->monster);
        se_list_exit(e->monster.status_effects);
        inv_exit(e->monster.inventory);
        free(e);
    }
    monster_list_initialised = false;
}

struct msr_monster *msrlst_get_next_monster(struct msr_monster *prev) {
    if (prev == NULL) {
        if (monster_list_head.tqh_first != NULL) return &monster_list_head.tqh_first->monster;
        return NULL;
    }
    struct msr_monster_list_entry *mle = container_of(prev, struct msr_monster_list_entry, monster);
    if (mle == NULL) return NULL;
    return &mle->entries.tqe_next->monster;
}

static uint32_t msrlst_next_id(void) {
    if (monster_list_initialised == false) msrlst_monster_list_init();
    struct msr_monster_list_entry *me = monster_list_head.tqh_first;
    uint32_t uid = 1;

    while (me != NULL) {
        if (uid <= me->monster.uid) uid = me->monster.uid+1;
        me = me->entries.tqe_next;
    }
    return uid;
}

struct msr_spawn_weigh_struct {
    int min_level;
    int max_level;
    enum dm_dungeon_type dt;
};

static int32_t msr_spawn_weight(void *ctx, int idx) {
    struct msr_spawn_weigh_struct *sws = ctx;

    if (sws->min_level <= static_monster_list[idx].spwn.level && sws->max_level >= static_monster_list[idx].spwn.level) {
        if (test_bf(static_monster_list[idx].spwn.dungeon_locale, sws->dt) ||
             test_bf(static_monster_list[idx].spwn.dungeon_locale, DUNGEON_TYPE_ALL) ) {
            return static_monster_list[idx].spwn.weight;
        }
    }

    return RANDOM_GEN_WEIGHT_IGNORE;
}

int32_t msr_spawn(int32_t roll, int min_level, int max_level, enum dm_dungeon_type dt) {
    struct msr_spawn_weigh_struct sws = {
        .min_level = min_level,
        .max_level = max_level,
        .dt = dt,
    };

    struct random_gen_settings s = {
        .start_idx = 0,
        .end_idx = ARRAY_SZ(static_monster_list),
        .roll = roll,
        .ctx = &sws,
        .weight = msr_spawn_weight,
    };

    return random_gen_spawn(&s);
}

#define MONSTER_PRE_CHECK (10477)
#define MONSTER_POST_CHECK (10706)
static void creature_weapon(struct msr_monster *monster);

struct msr_monster *msr_create(enum msr_ids tid) {
    if (tid >= (int) ARRAY_SZ(static_monster_list)) { assert(false && "template_id bigger than list"); return NULL; };
    if (tid == MID_NONE)                            { assert(false && "template_id invalid"); return NULL; };
    struct msr_monster *template_monster = &static_monster_list[tid];

    struct msr_monster_list_entry *m = calloc(1,sizeof(struct msr_monster_list_entry) );
    assert(m != NULL);

    memcpy(&m->monster, template_monster, sizeof(struct msr_monster) );
    m->monster.controller.ai.ai_ctx = NULL;
    m->monster.controller.controller_cb = NULL;
    m->monster.pos = cd_create(0,0);
    m->monster.uid = msrlst_next_id();
    m->monster.tid = tid;
    m->monster.energy = TT_ENERGY_FULL;
    m->monster.faction = MSR_FACTION_MONSTERS;
    m->monster.inventory = NULL;
    m->monster.status_effects = se_list_init();
    if (m->monster.description == NULL) {
        m->monster.description = "none";
    }
    assert(m->monster.description != NULL);

    for (int tier = 0; tier < MSR_TALENT_TIER_MAX; tier++) {
        m->monster.talents[tier] = 0;
        m->monster.talents[tier] |= MSR_TALENT_HEADER(tier);
    }

    m->monster.monster_pre = MONSTER_PRE_CHECK;
    m->monster.monster_post = MONSTER_POST_CHECK;

    switch (m->monster.race) {
        case MSR_RACE_HUMAN:
        case MSR_RACE_GREENSKIN:
            m->monster.inventory = inv_init(inv_loc_human);
            break;
        case MSR_RACE_BEAST:
        case MSR_RACE_DOMESTIC:
            m->monster.inventory = inv_init(inv_loc_animal);
            break;
        default:
            free(m);
            assert(false && "Unkown Race");
            return NULL;
    }

    creature_weapon(&m->monster);

    lg_debug("creating monster[%d, %s, %c]", m->monster.uid, m->monster.ld_name, m->monster.icon);

    TAILQ_INSERT_TAIL(&monster_list_head, m, entries);
    return &m->monster;
}

void msr_destroy(struct msr_monster *monster, struct dm_map *map) {
    if (msr_verify_monster(monster) == false) return;
    struct msr_monster_list_entry *target_mle = container_of(monster, struct msr_monster_list_entry, monster);

    if (map != NULL) {
        if (dm_verify_map(map) == false) return;
        msr_remove_monster(monster, map);
    }
    inv_exit(monster->inventory);
    se_list_exit(monster->status_effects);

    msr_clear_controller(monster);
    if (monster->unique_name != NULL) free(monster->unique_name);

    TAILQ_REMOVE(&monster_list_head, target_mle, entries);
    free(target_mle);
}

bool msr_verify_monster(struct msr_monster *monster) {
    assert(monster != NULL);
    assert(monster->monster_pre == MONSTER_PRE_CHECK);
    assert(monster->monster_post == MONSTER_POST_CHECK);
    assert(inv_verify_inventory(monster->inventory) == true );
    assert(se_verify_list(monster->status_effects) == true);

    for (unsigned int tier = 0; tier < MSR_TALENT_TIER_MAX; tier++) {
        assert(msr_get_tier(monster->talents[tier]) == tier);
    }

    return true;
}

struct msr_monster *msr_get_monster_by_uid(uint32_t uid) {
    if (monster_list_initialised == false) return NULL;
    struct msr_monster_list_entry *me = monster_list_head.tqh_first;

    while (me != NULL) {
        if (uid == me->monster.uid) return &me->monster;
        me = me->entries.tqe_next;
    }
    return NULL;
}

void msr_assign_controller(struct msr_monster *monster, struct monster_controller *controller) {
    if (msr_verify_monster(monster) == false) return;
    if (controller == NULL) return;
    memcpy(&monster->controller, controller, sizeof(struct monster_controller) );
}

void msr_clear_controller(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return;
    ai_monster_free(monster);
    memset(&monster->controller, 0x0, sizeof(struct monster_controller) );
}

bool msr_insert_monster(struct msr_monster *monster, struct dm_map *map, coord_t *pos) {
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(pos, &map->sett.size) == false) return false;

    struct dm_map_entity *me_future = dm_get_map_me(pos, map);
    if (TILE_HAS_ATTRIBUTE(me_future->tile, TILE_ATTR_TRAVERSABLE) == false) return false;
    if (me_future->monster != NULL) return false;

    me_future->monster = monster;
    monster->pos = *pos;
    lg_ai_debug(monster, "inserting \'%s\'(%c) to (%d,%d)",
                monster->sd_name, monster->icon, monster->pos.x, monster->pos.y);

    monster->idle_counter = 0;

    return true;
}

bool msr_move_monster(struct msr_monster *monster, struct dm_map *map, coord_t *pos) {
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(pos, &map->sett.size) == false) return false;
    if (cd_equal(&monster->pos, pos) == true ) return false;

    struct dm_map_entity *me_future = dm_get_map_me(pos, map);
    if (TILE_HAS_ATTRIBUTE(me_future->tile, TILE_ATTR_TRAVERSABLE) == false) return false;
    if (me_future->monster != NULL) return false;

    coord_t prev_pos = monster->pos;

    /*Speed of one for now*/
    if (cd_neighbour(&monster->pos, pos) == false) return false;

    assert(dm_tile_exit(map, &monster->pos, monster, pos) );
    if (msr_insert_monster(monster, map, pos) == false) return false;

    assert (dm_tile_enter(map, pos, monster, &prev_pos) );

    monster->idle_counter = 0;

    return true;
}
bool msr_give_item(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == true) return false;

    if (inv_add_item(monster->inventory, item) == true) {
        item->owner_type = ITEM_OWNER_MONSTER;
        item->owner.owner_monster = monster;
    }
    return true;
}

bool msr_remove_item(struct msr_monster *monster, struct itm_item *item) {
    if (msr_verify_monster(monster) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (inv_has_item(monster->inventory, item) == false) return false;

    if (inv_remove_item(monster->inventory, item) == true) {
        item->owner_type = ITEM_OWNER_NONE;
        item->owner.owner_monster = NULL;
    }
    return true;
}

int msr_get_near_sight_range(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;
    int sight_near = ( (msr_calculate_characteristic_bonus(monster, MSR_CHAR_PERCEPTION) * 1.0f) ) +1;
    if (msr_has_creature_trait(monster, CTRTRT_DARK_SIGHT) ) {
        int nv_sight = 16 * RANGE_MULTIPLIER;
        if(sight_near < nv_sight) sight_near = nv_sight;
    }

    if (se_has_effect(monster, EF_BLINDED) ) sight_near = 0;
    return sight_near;
}

int msr_get_medium_sight_range(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;
    int sight_medium = ( (msr_calculate_characteristic_bonus(monster, MSR_CHAR_PERCEPTION) * 2.0f) ) +1;
    if (msr_has_creature_trait(monster, CTRTRT_DARK_SIGHT) ) {
        int nv_sight = 16 * RANGE_MULTIPLIER;
        if(sight_medium < nv_sight) sight_medium = nv_sight;
    }

    if (se_has_effect(monster, EF_BLINDED) ) sight_medium = 0;
    return  sight_medium;
}

int msr_get_far_sight_range(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;
    int sight_far = ( (msr_calculate_characteristic_bonus(monster, MSR_CHAR_PERCEPTION) * 5.0f) ) +1;
    if (msr_has_creature_trait(monster, CTRTRT_DARK_SIGHT) ) {
        int nv_sight = 16 * RANGE_MULTIPLIER;
        if(sight_far < nv_sight) sight_far = nv_sight;
    }

    if (se_has_effect(monster, EF_BLINDED) ) sight_far = 0;
    return sight_far;
}

bool msr_drop_inventory(struct msr_monster *monster, struct dm_map *map) {
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;

    struct itm_item *item = NULL;
    while ( (item = inv_get_next_item(monster->inventory, item) ) != NULL) {
        if (itm_verify_item(item) == true) {
            if (inv_remove_item(monster->inventory, item) ) {
                if (itm_insert_item(item, map, &monster->pos) == false) {
                    itm_destroy(item);
                }
                item = NULL;
            }
        }
    }

    return true;
}

bool msr_remove_monster(struct msr_monster *monster, struct dm_map *map) {
    bool retval = false;
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;

    struct dm_map_entity *me_current = dm_get_map_me(&monster->pos, map);
    if (me_current->monster == monster) {
        lg_ai_debug(monster, "removed (%d,%d)", monster->pos.x, monster->pos.y);
        me_current->monster = NULL;
        monster->pos = cd_create(0,0);
        retval = true;
    }

    return retval;
}

int msr_get_energy(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;
    return monster->energy;
}

bool msr_change_energy(struct msr_monster *monster, int energy) {
    if (msr_verify_monster(monster) == false) return false;

    if (msr_calculate_carrying_capacity(monster) < inv_get_weight(monster->inventory) ) {
        se_add_status_effect(monster, SEID_ENCUMBERED, NULL);
    }

    monster->idle_counter += energy;
    //lg_ai_debug(monster, "idle counter: %d", monster->idle_counter);

    monster->energy += energy;
    //if (monster->energy < 0) monster->energy = 0;
    //if (monster->energy > TT_ENERGY_FULL) monster->energy = TT_ENERGY_FULL;
    //lg_ai_debug(monster, "energy: %d", monster->energy);

    return true;
}

struct itm_item *msr_get_armour_from_hitloc(struct msr_monster *monster, enum msr_hit_location mhl) {
    if (msr_verify_monster(monster) == false) return NULL;
    if (mhl >= MSR_HITLOC_MAX) return NULL;
    struct itm_item *item = NULL;

    switch(mhl) {
        case MSR_HITLOC_BODY:      item = inv_get_item_from_location(monster->inventory, INV_LOC_BODY); break;
        case MSR_HITLOC_LEFT_LEG:
        case MSR_HITLOC_RIGHT_LEG: item = inv_get_item_from_location(monster->inventory, INV_LOC_LEGS); break;
        case MSR_HITLOC_LEFT_ARM:
        case MSR_HITLOC_RIGHT_ARM: item = inv_get_item_from_location(monster->inventory, INV_LOC_ARMS); break;
        case MSR_HITLOC_HEAD:      item = inv_get_item_from_location(monster->inventory, INV_LOC_HEAD); break;
        default: break;
    }
    return item;
}

int msr_calculate_armour(struct msr_monster *monster, enum msr_hit_location mhl) {
    if (msr_verify_monster(monster) == false) return -1;
    if (mhl >= MSR_HITLOC_MAX) return 0;
    struct itm_item *item = msr_get_armour_from_hitloc(monster, mhl);
    int armour = 0;

    if (item != NULL) {
        if (wbl_is_type(item, WEARABLE_TYPE_ARMOUR) == true) {
            armour += item->specific.wearable.damage_reduction;
        }
    }

    /* Add Armour talents here */

    return armour;
}

bool msr_can_use_evasion(struct msr_monster *monster, enum msr_evasions evasion) {
    if (msr_verify_monster(monster) == false) return false;

    /* do status checks on monster*/
    if (se_has_effect(monster, EF_SWIMMING) ) return false;
    if (se_has_effect(monster, EF_STUNNED) ) return false;
    if (se_has_effect(monster, EF_BLINDED) ) return false;
    if (se_has_effect(monster, EF_PRONE) ) return false;

    struct inv_inventory *inv = monster->inventory;
    if (inv_verify_inventory(monster->inventory) == false) return false;

    switch(evasion) {
        case MSR_EVASION_MAIN_HAND: {
            struct itm_item *witem = inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD);
            if (wpn_is_type(witem, WEAPON_TYPE_MELEE) == false) return false;
            break; }
        case MSR_EVASION_OFF_HAND: {
            struct itm_item *witem = inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD);
            if (wpn_is_type(witem, WEAPON_TYPE_MELEE) == false) return false;
            break; }
        case MSR_EVASION_DODGE:
            break;
        case MSR_EVASION_MAX:
        default:
            return false;
    }

    return true;
}

bool msr_use_evasion(struct msr_monster *monster, struct msr_monster *attacker, struct itm_item *atk_wpn, enum msr_evasions evasion, int to_hit_DoS, int mod) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_can_use_evasion(monster, evasion) == false) return false;

    FIX_UNUSED(attacker);

    struct inv_inventory *inv = monster->inventory;
    if (inv_verify_inventory(inv) == false) return false;

    if (atk_wpn != NULL) {
        if (wpn_has_spc_quality(atk_wpn, WPN_SPCQLTY_FLEXIBLE) ) return false;
        if (wpn_has_spc_quality(atk_wpn, WPN_SPCQLTY_FAST) ) mod -= 10;
        if (wpn_has_spc_quality(atk_wpn, WPN_SPCQLTY_SLOW) ) mod += 10;
    }

    int roll = 0;
    switch(evasion) {
        case MSR_EVASION_MAIN_HAND: {
            struct itm_item *witem = inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD);
            if (witem != NULL) {
                if (wpn_has_spc_quality(witem, WPN_SPCQLTY_UNBALANCED) ) return false;
                if (wpn_has_spc_quality(witem, WPN_SPCQLTY_DEFENSIVE) ) mod += 5;
                if (wpn_has_spc_quality(witem, WPN_SPCQLTY_UNWIELDY) ) mod -= 5;
                roll = msr_skill_check(monster, MSR_SKILLS_PARRY, mod);
                Info("Using main-hand, unable to parry with it for one turn.");
            }
            break; }
        case MSR_EVASION_OFF_HAND: {
            struct itm_item *witem = inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD);
            if (witem != NULL) {
                if (wpn_has_spc_quality(witem, WPN_SPCQLTY_UNBALANCED) ) return false;
                if (wpn_has_spc_quality(witem, WPN_SPCQLTY_DEFENSIVE) ) mod += 5;
                if (wpn_has_spc_quality(witem, WPN_SPCQLTY_UNWIELDY) ) mod -= 5;
                roll = msr_skill_check(monster, MSR_SKILLS_PARRY, mod);
                Info("Using off-hand, unable to parry or attack with it for one turn.");
            }
            break; }
        case MSR_EVASION_DODGE: {
            roll = msr_skill_check(monster, MSR_SKILLS_DODGE, mod);
            Info("Using dodge, cannot dodge anymore for one turn.");
            break; }
        case MSR_EVASION_MAX:
        default:
            return false;
    }

    //msr_disable_evasion(monster, evasion);

    if (roll >= to_hit_DoS) {
        switch(evasion) {
            case MSR_EVASION_MAIN_HAND:
            case MSR_EVASION_OFF_HAND:
                You(monster, "defly parry the attack.");
                Monster(monster, "defly parries the attack.");
                break;
            case MSR_EVASION_DODGE:
                You(monster, "defly evade the attack.");
                Monster(monster, "defly evades the attack.");
                break;
            case MSR_EVASION_MAX:
            default: break;
        }
        return true;
    }

    return false;
}

enum msr_hit_location msr_get_hit_location(struct msr_monster *monster, int hit_roll) {
    if (msr_verify_monster(monster) == false) return MSR_HITLOC_NONE;
    if (hit_roll > 99) hit_roll %= 100;

    enum msr_hit_location mhl = MSR_HITLOC_NONE;
    int hitloc_tbl_sz = 0;
    const int *hitloc_tbl = NULL;

    switch (monster->race) {
        case MSR_RACE_GREENSKIN:
        case MSR_RACE_HUMAN:
            hitloc_tbl = human_hitloc_lotable;
            hitloc_tbl_sz = ARRAY_SZ(human_hitloc_lotable);
            break;
        case MSR_RACE_DOMESTIC:
        case MSR_RACE_BEAST:
            hitloc_tbl = beast_hitloc_lotable;
            hitloc_tbl_sz = ARRAY_SZ(beast_hitloc_lotable);
            break;
        default: assert(false && "Unkown Race"); break;
    }

    if (hitloc_tbl != NULL) {
        for (int i = 0; i < hitloc_tbl_sz; i++) {
            if (hit_roll >= hitloc_tbl[i]) mhl = hitloc_tbl[i];
        }
    }

    return mhl;
}

bool msr_die(struct msr_monster *monster, const char *origin, struct dm_map *map) {
    if (msr_verify_monster(monster) == false) return false;
    if (dm_verify_map(map) == false) return false;

    if (monster->is_player && (monster->fate_points > 0) ) {
        You(monster, "fall unconsious.");
        Monster(monster, "falls unconsious.");
    }
    else {
        You(monster, "are killed by %s", origin);
        Monster(monster, "is killed by %s", origin);

        msr_drop_inventory(monster, map);
        msr_remove_monster(monster, map);
    }

    if (gbl_game->player_data.player == monster) {
        gbl_game->player_data.career.killer = origin;
    }

    se_add_status_effect(monster, SEID_DEATH, origin);
    return true;
}

bool msr_do_dmg(struct msr_monster *monster, const char *origin, int dmg, enum dmg_type dmg_type, enum msr_hit_location mhl) {
    if (msr_verify_monster(monster) == false) return false;
    if (se_has_effect(monster, EF_DEAD) ) return false;
    assert(origin != NULL);
    bool was_critical = false;

     /* temp var so we can notify the player when the monster is critical wounded for the first time.*/
    if (monster->wounds.curr < 0) was_critical = true;

    if (dmg > 0) {
        monster->wounds.curr -= dmg;

        if (monster->wounds.curr < 0) {
            if (monster->unique_name == NULL) {
                if (dmg > MSR_WEAPON_DAMAGE_INSTA_DEATH) {
                    return msr_die(monster, origin, gbl_game->current_map);
                }
            }

            /* do critical hits! */
            if (mhl != MSR_HITLOC_NONE) {
                /* Add critical hit */
                se_add_critical_hit(monster, origin, monster->wounds.curr, mhl, dmg_type);
            }

            if (monster->wounds.curr < -STATUS_EFFECT_CRITICAL_MAX && (se_has_effect(monster, EF_DEAD) == false) ) {
                lg_ai_debug(monster, "Paranoia Death (%d, max %d).", monster->wounds.curr, -STATUS_EFFECT_CRITICAL_MAX);
                return msr_die(monster, origin, gbl_game->current_map);
            }

            if (was_critical == false) {
                //You(monster, "are criticly wounded.");
                Monster(monster, "is criticly wounded.");
            }
        }
        return true;
    }
    return false;
}

int msr_characteristic_check(struct msr_monster *monster, enum msr_characteristic chr, int mod) {
    if (msr_verify_monster(monster) == false) return false;
    int charac = msr_calculate_characteristic(monster, chr);
    assert(charac >= 0);
    lg_ai_debug(monster, "Characteristic is (%d)", charac);

    int con_mod = 0;
    con_mod += se_status_effect_strength(monster, EF_MODIFY_FATIQUE, -1);
    if (con_mod < 0) con_mod = 0;

    charac += mod + (con_mod * -10);
    lg_ai_debug(monster, "Check modifier is: %d (%d)", mod, charac + con_mod);

    int roll = (int) (random_int32(gbl_game->random)%100);
    int result = (charac - roll);
    int DoS = result / 10;
    if (roll < charac) DoS += 1;
    return DoS;
}

int msr_calculate_skill(struct msr_monster *monster, enum msr_skills skill) {
    if (msr_verify_monster(monster) == false) return false;

    int charac = msr_calculate_characteristic(monster, msr_skill_charac[skill]);
    assert(charac >= 0);
    lg_ai_debug(monster, "Characteristic is (%d)", charac);

    int con_mod = 0;
    con_mod += se_status_effect_strength(monster, EF_MODIFY_FATIQUE, -1);
    if (con_mod < 0) con_mod = 0;

    charac += (con_mod * -10);
    charac += se_status_effect_strength(monster, EF_MODIFY_SKILL, skill);
    lg_ai_debug(monster, "Conditions modifier is: %d (%d)", con_mod, charac);

    enum msr_skill_rate r = msr_has_skill(monster, skill);
    switch(r) {
        case MSR_SKILL_RATE_VETERAN:        charac += 30; lg_print("You are a veteran (%d)", charac); break;
        case MSR_SKILL_RATE_EXPERIENCED:    charac += 20; lg_print("You are an expert (%d)", charac); break;
        case MSR_SKILL_RATE_TRAINED:        charac += 10; lg_print("You are advanced (%d)", charac); break;
        case MSR_SKILL_RATE_KNOWN:          lg_print("You have the skill (%d)", charac); break;
        case MSR_SKILL_RATE_UNKNOWN:        charac = (charac / 2) +1; lg_print("You do not have this skill (%d)", charac); break;
        default: assert(false); break;
    }

    if (charac < 0) charac = 0;
    return charac;
}

int msr_skill_check(struct msr_monster *monster, enum msr_skills skill, int mod) {
    if (msr_verify_monster(monster) == false) return false;


    int skills = msr_calculate_skill(monster, skill) + mod;
    lg_print("Check is: %d (%d)", skills, mod);

    int roll = (random_int32(gbl_game->random)%100);
    int result = skills - roll;
    int DoS = result / 10;
    if (roll < skills) DoS += 1;

    lg_print("roll %d, result %d, DoS %d", roll, result, DoS);

    return DoS;
}

int msr_calculate_characteristic(struct msr_monster *monster, enum msr_characteristic chr) {
    if (msr_verify_monster(monster) == false) return -1;
    if (chr >= MSR_CHAR_MAX) return -1;
    int mod = 0;

    int adv = monster->characteristic[chr].advancement;
    int retchr = monster->characteristic[chr].base_value + adv + monster->characteristic[chr].mod + mod;
    if (retchr < 0) retchr = 0;
    return retchr;
}

int msr_calculate_characteristic_bonus(struct msr_monster *monster, enum msr_characteristic chr) {
    if (msr_verify_monster(monster) == false) return -1;
    if (chr >= MSR_CHAR_MAX)            return -1;

    return ( msr_calculate_characteristic(monster, chr) / 10);
}

int msr_calculate_fatique(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;

    /* TODO: implement fatique */

    return 0;
}

int msr_calculate_carrying_capacity(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return -1;

    int str = msr_calculate_characteristic(monster, MSR_CHAR_STRENGTH);
    int modifier = 10;
    return (str * modifier);
}

enum msr_skill_rate msr_has_skill(struct msr_monster *monster, enum msr_skills skill) {
    if (msr_verify_monster(monster) == false) return MSR_SKILL_RATE_UNKNOWN;

    enum msr_skill_rate r = MSR_SKILL_RATE_UNKNOWN;
    if ((monster->skills[MSR_SKILL_RATE_VETERAN]            & bf(skill) ) > 0) r = MSR_SKILL_RATE_VETERAN;
    else if ((monster->skills[MSR_SKILL_RATE_EXPERIENCED]   & bf(skill) ) > 0) r = MSR_SKILL_RATE_EXPERIENCED;
    else if ((monster->skills[MSR_SKILL_RATE_TRAINED]       & bf(skill) ) > 0) r = MSR_SKILL_RATE_TRAINED;
    else if ((monster->skills[MSR_SKILL_RATE_KNOWN]         & bf(skill) ) > 0) r = MSR_SKILL_RATE_KNOWN;
    return r;
}

bool msr_set_skill(struct msr_monster *monster, enum msr_skills skill, enum msr_skill_rate rate) {
    if (skill == MSR_SKILLS_NONE) return false;
    if (skill >= MSR_SKILLS_MAX) return false;
    if (rate >= MSR_SKILL_RATE_MAX) return false;
    if (msr_has_skill(monster, skill) == rate) return false;

    set_bf(monster->skills[rate], skill);

    return true;
}

const char *msr_gender_string(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return "nil";
    switch (monster->gender) {
        case MSR_GENDER_MALE: return "Male";
        case MSR_GENDER_FEMALE: return "Female";
        case MSR_GENDER_IT: return "None";
        default: break;
    }
    return "";
}

bool msr_weapons_check(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return false;
    if (monster->wpn_sel >= MSR_WEAPON_SELECT_MAX) return false;

    struct inv_inventory *inv = monster->inventory;

    /* check if the monster has any weapons */
    if ( (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) && (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) ) {
        if (inv_loc_empty(inv, INV_LOC_CREATURE_WIELD1) == true) return false;
        monster->wpn_sel = MSR_WEAPON_SELECT_CREATURE1;
    }

    /* If we have a single hand, test that for emptiness and weaponness. */
    if (monster->wpn_sel== MSR_WEAPON_SELECT_OFF_HAND) {
        /* if location is unsupported by this monster*/
        if (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == false) return false;
        /* if the location is empty */
        if (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) return false;
        /* if there is no weapon at that location*/
        if (inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;
        /* or if it is the same as the mainhand weapon */
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD) == inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD) ) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_MAIN_HAND) {
        if (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) return false;
        if (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD) == inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD) ) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND) {
        if (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) return false;
        if (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == false) return false;

        if (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;
        if (inv_loc_empty(inv, INV_LOC_OFFHAND_WIELD) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;

        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD) == inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD) ) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) {
        if (inv_support_location(inv, INV_LOC_MAINHAND_WIELD) == false) return false;
        if (inv_support_location(inv, INV_LOC_OFFHAND_WIELD) == false) return false;

        if (inv_loc_empty(inv, INV_LOC_MAINHAND_WIELD) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD)->item_type != ITEM_TYPE_WEAPON) return false;

        if ( (wpn_is_catergory(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), WEAPON_CATEGORY_2H_RANGED) == false) &&
             (wpn_is_catergory(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), WEAPON_CATEGORY_2H_MELEE) == false) ) {
            return false;
        }
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_CREATURE1) {
        if (inv_support_location(inv, INV_LOC_CREATURE_WIELD1) == false) return false;
        if (inv_loc_empty(inv, INV_LOC_CREATURE_WIELD1) == true) return false;
        if (inv_get_item_from_location(inv, INV_LOC_CREATURE_WIELD1)->item_type != ITEM_TYPE_WEAPON) return false;
    }
    else return false;

    return true;
}

bool msr_weapon_type_check(struct msr_monster *monster, enum item_weapon_type type) {
    if (msr_weapons_check(monster) == false) return false;
    if (type >= WEAPON_TYPE_MAX) return false;
    struct inv_inventory *inv = monster->inventory;

    if (monster->wpn_sel == MSR_WEAPON_SELECT_OFF_HAND) {
         if (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD), type) == false) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_MAIN_HAND) {
         if (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), type) == false) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_DUAL_HAND) {
         if ( (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_OFFHAND_WIELD), type) == false) &&
              (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), type) == false) ) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_BOTH_HAND) {
         if (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_MAINHAND_WIELD), type) == false) return false;
    }
    else if (monster->wpn_sel == MSR_WEAPON_SELECT_CREATURE1) {
         if (wpn_is_type(inv_get_item_from_location(inv, INV_LOC_CREATURE_WIELD1), type) == false) return false;
    }
    else return false;


    return true;
}

bool msr_weapon_next_selection(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return false;

    if ( (inv_loc_empty(monster->inventory, INV_LOC_MAINHAND_WIELD) == true) &&
         (inv_loc_empty(monster->inventory, INV_LOC_OFFHAND_WIELD) == true) ) {
        monster->wpn_sel = MSR_WEAPON_SELECT_CREATURE1;
        return false;
    }

    bool first_round = true;
    do {
        monster->wpn_sel++;
        monster->wpn_sel %= MSR_WEAPON_SELECT_MAX;

        /* Prevent unarmed from being selected when it is not neccesary */
        if (monster->wpn_sel == MSR_WEAPON_SELECT_CREATURE1 && first_round) {
            first_round = false;
            continue;
        };
    } while (msr_weapons_check(monster) == false);
    return true;
}

static void creature_weapon(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return;
    assert(monster->crtr_wpn != IID_NONE);
    if (monster->crtr_wpn == IID_NONE) return;
    struct itm_item *item = itm_create(monster->crtr_wpn);

    if (itm_verify_item(item) == true) {
        assert(itm_is_type(item, ITEM_TYPE_WEAPON) );

        if (inv_add_item(monster->inventory, item) == true) {
            if (wpn_has_spc_quality(item, WPN_SPCQLTY_CREATURE) ) {
                if (inv_loc_empty(monster->inventory, INV_LOC_CREATURE_WIELD1) ) {
                    assert(inv_move_item_to_location(monster->inventory, item, INV_LOC_CREATURE_WIELD1) );
                }
            }
        }
    }

    assert(inv_loc_empty(monster->inventory, INV_LOC_CREATURE_WIELD1) == false);
}

void msr_populate_inventory(struct msr_monster *monster, int lvl_min, int lvl_max, struct random *r) {
    if (msr_verify_monster(monster) == false) return;
    struct itm_item *item = NULL;

    int range = lvl_max - lvl_min;
    int mlevel = (random_int32(r) % range) + lvl_min;
    if ( (random_int32(r) % 100) > 98) mlevel += random_int32(r) % 3;

    for (int i = 0; i < MSR_NR_DEFAULT_WEAPONS_MAX; i++) {
        if (monster->def_items[i] != ITEM_GROUP_NONE) {
            item = itm_create(itm_spawn(random_int32(r), mlevel, monster->def_items[i], monster) );
            if (itm_verify_item(item) == true) {
                if (inv_add_item(monster->inventory, item) == true) {
                    if (dw_can_wear_item(monster, item) ) {
                        assert(dw_wear_item(monster, item) == true);
                    }

                    if (wpn_uses_ammo(item) ) {
                        enum item_ids ammo_id = wpn_get_ammo_used_id(item);
                        struct itm_item *ammo = itm_create(ammo_id);
                        int nr = (random_int32(gbl_game->random) % (ammo->max_quantity / 5) ) +10;
                        ammo->stacked_quantity = nr;
                        assert (inv_add_item(monster->inventory, ammo) == true);
                    }
                }
            }
        }
    }

    assert(inv_loc_empty(monster->inventory, INV_LOC_CREATURE_WIELD1) == false);
}

bool msr_has_creature_trait(struct msr_monster *monster,  enum msr_creature_traits trait) {
    if (msr_verify_monster(monster) == false) return false;
    return ( (monster->creature_traits & bf(trait) ) > 0);
}

bool msr_set_creature_trait(struct msr_monster *monster,  enum msr_creature_traits trait) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_has_creature_trait(monster, trait) == true) return false;
    return monster->creature_traits |= bf(trait);
}

bool msr_clr_creature_trait(struct msr_monster *monster,  enum msr_creature_traits trait) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_has_creature_trait(monster, trait) == false) return false;
    monster->creature_traits &= ~bf(trait);

    return msr_has_creature_trait(monster, trait);
}

enum msr_talent_tiers msr_get_tier(bitfield64_t t) {
    bitfield64_t header = t>> MSR_TALENT_HEADER_SHIFT;
    assert(__builtin_popcountll(header) == 1);

    for (int i = 0; i < MSR_TALENT_TIER_MAX; i++)  {
        if (header == 1) return i;
        header >>= 1;
    }
    return MSR_TALENT_TIER_MAX;
}

bool msr_has_talent(struct msr_monster *monster, enum msr_talents talent) {
    if (msr_verify_monster(monster) == false) return false;
    if (talent == TLT_NONE) return true;
    if (talent >= TLT_MAX) return false;

    unsigned int tier = msr_get_tier(talent);
    assert(tier < MSR_TALENT_TIER_MAX);
    assert(msr_get_tier(monster->talents[tier]) == tier);
    assert(__builtin_popcountll(talent >> MSR_TALENT_HEADER_SHIFT) == 1);

    bitfield64_t talent_id = talent & MSR_TALENT_ID_MASK;
    assert(__builtin_popcountll(talent_id) == 1);

    if ( (monster->talents[tier] & talent_id) == talent_id) return true;
    return false;
}

bool msr_set_talent(struct msr_monster *monster, enum msr_talents talent) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_has_talent(monster, talent) == true) return false;
    if (talent >= TLT_MAX) return false;

    unsigned int tier = msr_get_tier(talent);
    assert(tier < MSR_TALENT_TIER_MAX);
    assert(__builtin_popcountll(talent >> MSR_TALENT_HEADER_SHIFT) == 1);

    bitfield64_t talent_id = talent & MSR_TALENT_ID_MASK;
    assert(__builtin_popcountll(talent_id) == 1);

    monster->talents[tier] |= talent;
    assert(msr_get_tier(monster->talents[tier]) == tier);

    return msr_has_talent(monster, talent);
}

bool msr_clr_talent(struct msr_monster *monster, enum msr_talents talent) {
    if (msr_verify_monster(monster) == false) return false;
    if (msr_has_talent(monster, talent) == false) return false;

    int tier = msr_get_tier(talent);
    assert(tier < MSR_TALENT_TIER_MAX);
    assert(__builtin_popcountll(talent >> MSR_TALENT_HEADER_SHIFT) == 1);

    bitfield64_t talent_id = talent & MSR_TALENT_ID_MASK;
    assert(__builtin_popcountll(talent_id) == 1);

    monster->talents[tier] &= ~talent;
    return (msr_has_talent(monster, talent) == false);
}

uint8_t msr_get_movement_rate(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return false;
    int speed = (msr_calculate_characteristic_bonus(monster, MSR_CHAR_AGILITY) );
    int speed_mod = 0;
    int min_speed = MSR_MOVEMENT_MIN;
    int max_speed = MSR_MOVEMENT_MAX;

    if ( (se_has_effect(monster, EF_DISABLED_RLEG) == true) &&
         (se_has_effect(monster, EF_DISABLED_LLEG) == true) ) {
        return 0;
    }

    if (se_has_effect(monster, EF_DISABLED_RLEG) ) {
        max_speed -= 2;
        speed_mod -= speed / 2;
    }
    if (se_has_effect(monster, EF_DISABLED_LLEG) ) {
        max_speed -= 2;
        speed_mod -= speed / 2;
    }

    speed += speed_mod;
    if (speed < min_speed) speed = min_speed;
    speed -= se_status_effect_strength(monster, EF_ENCUMBERED, -1);
    if (speed > max_speed) speed = max_speed;
    if (speed < 0) speed = 0;

    lg_ai_debug(monster, "speed is (%d)", speed);
    return speed;
}

struct ai *msr_get_ai_ctx(struct msr_monster *monster) {
    return &monster->controller.ai;
}

const char *msr_ldname(struct msr_monster *monster) {
    if (msr_verify_monster(monster) == false) return cs_MONSTER "unknown" cs_CLOSE;

    if (monster->is_player) return cs_PLAYER "you" cs_CLOSE;
    if (!dm_get_map_me(&monster->pos, gbl_game->current_map)->visible) return cs_MONSTER "something" cs_CLOSE;
    return monster->ld_name;
}

const char *msr_gender_name(struct msr_monster *monster, bool possesive) {
    if (msr_verify_monster(monster) == false) return cs_MONSTER "unknown" cs_CLOSE;

    enum msr_gender gender = monster->gender;
    if (monster->gender >= MSR_GENDER_MAX) gender = MSR_GENDER_IT;
    if (!dm_get_map_me(&monster->pos, gbl_game->current_map)->visible) gender = MSR_GENDER_IT;

    int pos = (possesive) ? 0 : 1;
    return gender_names[gender][pos];
}

const char *msr_char_names(enum msr_characteristic c) {
    if (c >= MSR_CHAR_MAX) return NULL;
    return msr_char_name[c];
}

const char *msr_char_descriptions(enum msr_characteristic c) {
    if (c >= MSR_CHAR_MAX) return NULL;
    return msr_char_description[c];
}

const char *msr_skill_names(enum msr_skills s) {
    if (s >= MSR_SKILLS_MAX) return NULL;
    return msr_skill_name[s];
}

const char *msr_skill_descriptions(enum msr_skills s) {
    if (s >= MSR_SKILLS_MAX) return NULL;
    return msr_skill_description[s];
}

const char *msr_skillrate_names(enum msr_skill_rate sr) {
    if (sr < 0) return NULL;
    if (sr >= MSR_SKILL_RATE_MAX) return NULL;
    return msr_skillrate_name[sr];
}

const char *msr_talent_names(enum msr_talents t) {
    if (t >= TLT_MAX) return NULL;
    for (unsigned int i = 0; i < ARRAY_SZ(talent_descriptions); i++) {
        if (talent_descriptions[i].talent != t) continue;

        return talent_descriptions[i].name;
    }
    return NULL;
}

const char *msr_talent_descriptions(enum msr_talents t) {
    if (t >= TLT_MAX) return NULL;
    for (unsigned int i = 0; i < ARRAY_SZ(talent_descriptions); i++) {
        if (talent_descriptions[i].talent != t) continue;

        return talent_descriptions[i].description;
    }
    return NULL;
}

const char *msr_hitloc_name(struct msr_monster *monster, enum msr_hit_location mhl) {
    if (msr_verify_monster(monster) == false) return NULL;
    if (mhl >= MSR_HITLOC_MAX) return NULL;

    return hitloc_names[mhl];
}

void msr_dbg_check_all() {
    struct msr_monster *monster = msrlst_get_next_monster(NULL);
    while ( (monster = msrlst_get_next_monster(monster) ) != NULL) {
        msr_verify_monster(monster);
    }
}

