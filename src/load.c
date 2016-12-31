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

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "logging.h"
#include "load.h"
#include "save.h"
#include "options.h"
#include "input.h"
#include "coord.h"
#include "player.h"
#include "random.h"
#include "inventory.h"
#include "monster/monster.h"
#include "status_effects/status_effects.h"
#include "status_effects/ground_effects.h"
#include "items/items.h"
#include "dungeon/tiles.h"
#include "dungeon/dungeon_map.h"
#include "ai/ai.h"
#include "careers/careers.h"
#include "quests/quests.h"

/**
* Evaluates a Lua expression and returns the string result.
* If an error occurs or the result is not string, def is returned.
*
* @param L the lua_State in which the string resides.
* @param expr the lua expression to retreive the string
* @param def the default string should the State not contain the expression given
*
* @return a pointer towards the asked string.
*/
static const char* lua_stringexpr(lua_State *L, const char* format, ...)
{
    const char *r = NULL;
    char buf0[230] = "";
    char buf[256] = "";

    va_list args;
    va_start(args, format);
    (void) vsnprintf(buf0, 227, format, args);
    va_end(args);

    /* Assign the Lua expression to a Lua global variable. */
    (void) snprintf(buf, 255, "evalExpr=%s", buf0);

    if (luaL_dostring(L, buf) == 0)
    {
        /* Get the value of the global varibable */
        lua_getglobal(L, "evalExpr");

        if (lua_isstring(L, -1) == 1)
        {
            r = lua_tostring(L, -1);
        }

        /* remove lua_getglobal value */
        lua_pop(L, 1);
    }
    return r;
}

/**
* Evaluates a Lua expression and returns the number result.
*
* @param L the lua_State in which the number resides.
* @param expr the lua expression to retreive the number
* @param out the result of the expression.
*
* @return zero on succes, otherwise 1.
*/
static int lua_intexpr_va(lua_State *L, ptrdiff_t *out, const char* format, va_list args)
{
    int ok = 0;
    char buf[256] = "";
    char expr[256] = "";

    /* Assign the Lua expression to a Lua global variable. */
    (void) vsnprintf(expr, sizeof(expr), format, args);
    (void) snprintf(buf, 255, "evalExpr=%s", expr);

    if (luaL_dostring(L, buf) == 0)
    {
        /* Get the value of the global varibable */
        lua_getglobal(L, "evalExpr");

        if (lua_isnumber(L, -1) == 1)
        {
            *out = lua_tointeger(L, -1);
            ok = 1;
        }

        /* remove lua_getglobal value */
        lua_pop(L, 1);
    }

    return ok;
}

/**
* Evaluates a Lua expression and returns the int result.
* This uses lua_numberexpr underwater.
*
* @param L the lua_State in which the int resides.
* @param expr the lua expression to retreive the int
* @param out the result of the expression
*
* @return zero on succes, otherwise 1.
*/
static int lua_intexpr(lua_State* L, uint64_t *out, const char* format, ...)
{
    ptrdiff_t d = 0;
    int ok = 1;

    va_list args;
    va_start(args, format);
    ok = lua_intexpr_va(L, &d, format, args);
    va_end(args);

    if (ok == 1)
    {
        *out = (uint64_t) d;
    }

    return ok;
}

#if 0 /* Not used atm. */
/**
* Evaluates a Lua expression and returns the bool result.
* If an error occurs or the result is not bool, def is returned.
*
* @param L the lua_State in which the bool resides.
* @param expr the lua expression to retreive the bool
* @param def the default bool should the State not contain the expression given
*
* @return the bool retreived from lua via the expression
*/
static int lua_boolexpr(lua_State* L, bool def, const char* format, va_list args)
{
    bool ok = def;
    char buf[256] = "";
    char expr[256] = "";

    (void) vsnprintf(expr, sizeof(expr), format, args);
    /* Assign the Lua expression to a Lua global variable. */
    (void) snprintf(buf, 255, "evalExpr=%s", expr);

    if (luaL_dostring(L, buf) == 0)
    {
        /* Get the value of the global varibable */
        lua_getglobal(L, "evalExpr");

        if (lua_isboolean(L, -1) == true)
        {
            int t = lua_toboolean(L, -1);
            ok = (bool) t;
        }

        /* remove lua_getglobal value */
        lua_pop(L, 1);
    }

    return ok;
}
#endif

/**
* Opens the given file, and executes it within a Lua Context.
*
* @param file A path and file towards the config file.
*
* @return returns the lua_State on succes or NULL otherwise.
*/
static lua_State *conf_open(const char *file)
{
    int err = 0;
    lua_State *L = luaL_newstate();
    if (L == NULL)
    {
        lg_error("load", "Bailing out! No memory.");
        return L;
    }

    luaL_openlibs(L);
    if ( (err = luaL_loadfile(L, file) ) != 0)
    {
        if (err == LUA_ERRFILE)
        {
            lg_debug("cannot access config file: %s", file);
        }
        else if (err == LUA_ERRSYNTAX)
        {
            lg_debug("syntax error in config file: %s", file);
        }
        else if (err == LUA_ERRERR)
        {
            lg_debug("general LUA error when accessing config file: %s", file);
        }
        else if (err == LUA_ERRMEM)
        {
            lg_debug("out of memory when accessing config file: %s", file);
        }
        else
        {
            lg_debug("Unkown LUA error(%d) when accessing config file: %s", err, file);
        }
        lua_close(L);
        L = NULL;
    }
    else
    {
        if ( (err = lua_pcall(L, 0, LUA_MULTRET, 0) ) != 0)
        {
            if (err == LUA_ERRRUN)
            {
                lg_debug("Run error when executing config file: %s", file);
            }
            else if (err == LUA_ERRERR)
            {
                lg_debug("general LUA error when executing config file: %s", file);
            }
            else if (err == LUA_ERRMEM)
            {
                lg_debug("out of memory when executing config file: %s", file);
            }
            else
            {
                lg_debug("unkown error(%d) when executing config file: %s", err, file);
            }

            lua_close(L);
            L = NULL;
        }
        else lg_debug("loaded config file in memory; parsing now...");
    }

    return L;
}

static bool load_game(lua_State *L, struct gm_game *g) {
    uint64_t t;
    if (L == NULL) return false;

    const char *version_ptr;
    if ( (version_ptr = lua_stringexpr(L,"game.git_version") ) == NULL) return false;
    if (strcmp(version_ptr, GIT_VERSION) != 0) lg_warning("Warning: save game version and game version do not match!");

    if (lua_intexpr(L, &t, "game.random.seed") == 0) return false;
    g->random = random_init_genrand(t);

    if (options.play_recording == false) {
        if (lua_intexpr(L, &t, "game.turn") == 0) return false;
        g->turn = t;

        if (lua_intexpr(L, &t, "game.random.called") == 0) return false;
        random_loop_called(g->random, t);
    }

    return true;
}

static bool load_log(lua_State *L, struct logging *lctx) {
    uint64_t t;
    if (L == NULL) return false;

    if (lua_intexpr(L, &t, "game.log.sz") == 0) return false;

    int sz = t;
    for (int i = 0; i < sz; i++) {
        struct log_entry le;
        lua_intexpr(L, &t, "game.log[%d].turn",     i+1); le.turn   = t;
        lua_intexpr(L, &t, "game.log[%d].repeated", i+1); le.repeat = t;
        lua_intexpr(L, &t, "game.log[%d].level",    i+1); le.level  = t;
        lua_intexpr(L, &t, "game.log[%d].line",     i+1); le.line   = t;

        if (le.level <= LG_DEBUG_LEVEL_GAME_INFO) {
            /*
            const char *mod_ptr = lua_stringexpr(L, "game.log[%d].module", i+1);
            if (mod_ptr != NULL) {
                le.module = strdup(mod_ptr);
            }
            */
            le.module = strdup("");

            const char *str_ptr = lua_stringexpr(L, "game.log[%d].string", i+1);
            if (str_ptr != NULL) {
                le.string = strdup(str_ptr);
            }

            lg_add_entry(lctx, &le);
        }
    }

    return true;
}

static bool load_input(lua_State *L, struct gm_game *g) {
    uint64_t t;
    if (L == NULL) return false;

    if (lua_intexpr(L, &t, "game.input.keylog.sz") == 0) return false;

    int sz = t;
    for (int i = 0; i < sz; i++) {
        lua_intexpr(L, &t, "game.input.keylog[%d]", i+1); inp_add_to_log(g->input, t);
    }

    g->input->keylog_widx = sz;
    g->input->keylog_ridx = sz;
    if (options.play_recording) g->input->keylog_ridx = 0;

    lg_debug("keylog size: %d", sz);

    return true;
}

static bool load_player(lua_State *L, struct pl_player *plr) {
    uint64_t t;
    if (L == NULL) return false;

    lua_intexpr(L, &t, "game.player.xp_spend");     plr->xp_spend = t;
    lua_intexpr(L, &t, "game.player.xp_current");   plr->xp_current = t;
    lua_intexpr(L, &t, "game.player.career_id");    plr->career = cr_get_career_by_id(t);

    lua_intexpr(L, &t, "game.player.quest.tid");    plr->quest = qst_by_tid(t);
    lua_intexpr(L, &t, "game.player.quest.state");  plr->quest->state = t;
    lua_intexpr(L, &t, "game.player.quest.params.sz");
    int params_sz = t;
    for (int i = 0; i < params_sz; i++) {
        lua_intexpr(L, &t, "game.player.quest.params[%d]", i+1); plr->quest->params[i] = t;
    }
    return true;
}

static bool load_items_list(lua_State *L) {
    uint64_t t;
    if (L == NULL) return false;
    if (lua_intexpr(L, &t, "game.items.sz") == 0) return false;

    int items_sz = t;
    for (int i = 0; i < items_sz; i++) {
        if (lua_intexpr(L, &t, "game.items[%d].tid", i+1) == 0) return false;
        int tid = t;
        struct itm_item *item = itm_create(tid);
        if (itm_verify_item(item) == false) return false;

        lua_intexpr(L, &t, "game.items[%d].uid", i+1); item->uid = t;
        lua_intexpr(L, &t, "game.items[%d].quality", i+1); item->quality = t;
        lua_intexpr(L, &t, "game.items[%d].quantity", i+1); item->stacked_quantity = t;
        lua_intexpr(L, &t, "game.items[%d].identified", i+1); item->identified = t;

        switch(item->item_type) {
            case ITEM_TYPE_WEAPON: {
                    struct item_weapon_specific *wpn = &item->specific.weapon;
                    lua_intexpr(L, &t, "game.items[%d].weapon.magazine_left", i+1); wpn->magazine_left = t;
                    lua_intexpr(L, &t, "game.items[%d].weapon.special_quality", i+1); wpn->special_quality = t;
                    lua_intexpr(L, &t, "game.items[%d].weapon.upgrades", i+1); wpn->upgrades = t;
                    lua_intexpr(L, &t, "game.items[%d].weapon.rof_set", i+1); wpn->rof_set = t;
                    lua_intexpr(L, &t, "game.items[%d].weapon.jammed", i+1); wpn->jammed = t;
                    lua_intexpr(L, &t, "game.items[%d].weapon.ammo_used_tid", i+1); wpn->ammo_used_tid = t;
                    if (wpn_ranged_weapon_rof_set_check(item) == false) wpn_ranged_weapon_rof_set_check(item);
                } break;
            case ITEM_TYPE_WEARABLE: {
                    /*struct item_wearable_specific *wear = &item->specific.wearable;
                    fprintf(file, "wearable={},");*/
                } break;
            case ITEM_TYPE_FOOD: {
                    struct item_food_specific *food = &item->specific.food;
                    lua_intexpr(L, &t, "game.items[%d].food.nutrition_left", i+1); food->nutrition_left = t;
                    lua_intexpr(L, &t, "game.items[%d].food.side_effect_chance", i+1); food->side_effect_chance = t;
                    lua_intexpr(L, &t, "game.items[%d].food.side_effect", i+1); food->side_effect = t;
                } break;
            case ITEM_TYPE_TOOL: {
                    struct item_tool_specific *tool = &item->specific.tool;
                    lua_intexpr(L, &t, "game.items[%d].tool.lit", i+1); tool->lit = t;
                    lua_intexpr(L, &t, "game.items[%d].tool.energy", i+1); tool->energy = t;
                } break;
            case ITEM_TYPE_AMMO: {
                } break;
            default: break;
        }
    }
    return true;
}

static bool load_status_effect_list(lua_State *L) {
    uint64_t t;
    if (L == NULL) return false;

    if (lua_intexpr(L, &t, "game.status_effects.sz") >= 1) {
        int status_effect_sz = t;
        for (int j = 0; j < status_effect_sz; j++) {
            if (lua_intexpr(L, &t, "game.status_effects[%d].tid", j+1) == 1) {
                struct status_effect *c = se_create(t);
                if (c != NULL) {
                    lua_intexpr(L, &t, "game.status_effects[%d].uid", j+1); c->uid = t;
                    lua_intexpr(L, &t, "game.status_effects[%d].duration_energy_min", j+1); c->duration_energy_min = t;
                    lua_intexpr(L, &t, "game.status_effects[%d].duration_energy_max", j+1); c->duration_energy_max = t;
                    lua_intexpr(L, &t, "game.status_effects[%d].duration_energy", j+1);     c->duration_energy = t;

                    lua_intexpr(L, &t, "game.status_effects[%d].effects.sz", j+1);
                    int effects_sz = t;
                    for (int e = 0; e < effects_sz; e++) {
                        c->effects[e].effect = EF_NONE;
                        c->effects[e].effect_setting_flags = 0;

                        if (e < effects_sz) {
                            lua_intexpr(L, &t, "game.status_effects[%d].effects[%d].effect", j+1, e+1);
                                                    c->effects[e].effect = t;
                            lua_intexpr(L, &t, "game.status_effects[%d].effects[%d].effect_setting_flags", j+1, e+1);
                                                    c->effects[e].effect_setting_flags = t;
                            lua_intexpr(L, &t, "game.status_effects[%d].effects[%d].tick_interval_energy", j+1, e+1);
                                                    c->effects[e].tick_interval_energy = t;
                            lua_intexpr(L, &t, "game.status_effects[%d].effects[%d].tick_energy", j+1, e+1);
                                                    c->effects[e].tick_energy = t;
                            lua_intexpr(L, &t, "game.status_effects[%d].effects[%d].ticks_applied", j+1, e+1);
                                                    c->effects[e].ticks_applied = t;
                            lua_intexpr(L, &t, "game.status_effects[%d].effects[%d].param", j+1, e+1);
                                                    c->effects[e].param = t;
                        }
                    }
                }
            }
        }
    }

    if (lua_intexpr(L, &t, "game.ground_effects.sz") >= 1) {
        int ground_effect_sz = t;
        for (int j = 0; j < ground_effect_sz; j++) {
            if (lua_intexpr(L, &t, "game.ground_effects[%d].tid", j+1) == 1) {
                enum ge_ids tid = t;
                coord_t c;
                lua_intexpr(L, &t, "game.ground_effects[%d].pos.x", j+1); c.x = t;
                lua_intexpr(L, &t, "game.ground_effects[%d].pos.y", j+1); c.y = t;

                struct dm_map_entity *me = dm_get_map_me(&c, gbl_game->current_map);
                struct ground_effect *ge = ge_create(tid, me);
                if (ge != NULL) {
                    lua_intexpr(L, &t, "game.ground_effects[%d].uid", j+1);             ge->uid = t;
                    lua_intexpr(L, &t, "game.ground_effects[%d].min_energy", j+1);      ge->min_energy = t;
                    lua_intexpr(L, &t, "game.ground_effects[%d].max_energy", j+1);      ge->max_energy = t;
                    lua_intexpr(L, &t, "game.ground_effects[%d].current_energy", j+1);  ge->current_energy = t;
                    lua_intexpr(L, &t, "game.ground_effects[%d].se_id", j+1);           ge->current_energy = t;
                }
            }
        }
    }

    return true;
}

static bool load_monsters(lua_State *L, struct dm_map *map, struct gm_game *g) {
    uint64_t t;
    if (L == NULL) return false;
    if (lua_intexpr(L, &t, "game.monsters.sz") == 0) return false;

    uint32_t leader_uid;

    int monsters_sz = t;
    for (int i = 0; i < monsters_sz; i++) {
        if (lua_intexpr(L, &t, "game.monsters[%d].tid", i+1) == 0) return false;
        struct msr_monster *monster = msr_create(t);
        if (msr_verify_monster(monster) == false) return false;


        monster->tid = t;
        lua_intexpr(L, &t, "game.monsters[%d].uid", i+1); monster->uid = t;
        lua_intexpr(L, &t, "game.monsters[%d].race", i+1); monster->race = t;
        lua_intexpr(L, &t, "game.monsters[%d].size", i+1); monster->size = t;
        lua_intexpr(L, &t, "game.monsters[%d].gender", i+1); monster->gender = t;
        lua_intexpr(L, &t, "game.monsters[%d].fate_points", i+1); monster->fate_points = t;
        lua_intexpr(L, &t, "game.monsters[%d].insanity_points", i+1); monster->insanity_points = t;
        lua_intexpr(L, &t, "game.monsters[%d].corruption_points", i+1); monster->corruption_points = t;

        lua_intexpr(L, &t, "game.monsters[%d].idle_counter", i+1); monster->idle_counter = t;
        lua_intexpr(L, &t, "game.monsters[%d].ai_leader", i+1); leader_uid = t;

        lua_intexpr(L, &t, "game.monsters[%d].creature_traits", i+1); monster->creature_traits = t;
        lua_intexpr(L, &t, "game.monsters[%d].is_player", i+1); monster->is_player = t;
        lua_intexpr(L, &t, "game.monsters[%d].pos.x", i+1); monster->pos.x = t;
        lua_intexpr(L, &t, "game.monsters[%d].pos.y", i+1); monster->pos.y = t;

        lua_intexpr(L, &t, "game.monsters[%d].wounds.curr", i+1); monster->wounds.curr = t;
        lua_intexpr(L, &t, "game.monsters[%d].wounds.max", i+1); monster->wounds.max = t;
        lua_intexpr(L, &t, "game.monsters[%d].wounds.added", i+1); monster->wounds.added = t;

        const char *name_ptr = lua_stringexpr(L, "game.monsters[%d].unique_name", i+1);
        if (name_ptr != NULL) {
            monster->unique_name = strdup(name_ptr);
            lg_debug("monster[%d] name is %s", monster->uid, monster->unique_name);
        }

        if (lua_intexpr(L, &t, "game.monsters[%d].evasion.sz", i+1) == 1) {
            int evasion_sz = t;
            for (int j = 0; j < evasion_sz; j++) {
                if (lua_intexpr(L, &t, "game.monsters[%d].evasion[%d]", i+1, j+1) == 1) {
                    monster->evasion_last_used[j] = t;
                }
            }
        }


        if (lua_intexpr(L, &t, "game.monsters[%d].skills.sz", i+1) == 1) {
            int skills_sz = t;
            for (int j = 0; j < skills_sz; j++) {
                if (lua_intexpr(L, &t, "game.monsters[%d].skills[%d]", i+1, j+1) == 1) {
                    monster->skills[j] = t;
                }
            }
        }

        if (lua_intexpr(L, &t, "game.monsters[%d].status_effects.sz", i+1) == 1) {
            int status_effect_sz = t;
            for (int j = 0; j < status_effect_sz; j++) {
                if (lua_intexpr(L, &t, "game.monsters[%d].status_effects[%d]", i+1, j+1) == 1) {
                    struct status_effect *c = selst_status_effect_by_uid(t);
                    if (c != NULL) {
                        se_add_to_list(monster, c);
                    }
                }
            }
        }

        if (lua_intexpr(L, &t, "game.monsters[%d].talents.sz", i+1) == 1) {
            int talents_sz = t;
            for (int j = 0; j < MSR_NR_TALENTS_MAX; j++) {
                monster->talents[j] = TLT_NONE;

                if (j < talents_sz) {
                    if (lua_intexpr(L, &t, "game.monsters[%d].talents[%d]", i+1, j+1) == 1) {
                        monster->talents[j] = t;
                    }
                }
            }
        }

        if (lua_intexpr(L, &t, "game.monsters[%d].characteristic.sz", i+1) == 1) {
            int characteristic_sz = t;
            for (int j = 0; j < characteristic_sz; j++) {
                if (lua_intexpr(L, &t, "game.monsters[%d].characteristic[%d].base_value", i+1,j+1) == 1) {
                    monster->characteristic[j].base_value = t;
                    lua_intexpr(L, &t, "game.monsters[%d].characteristic[%d].advancement", i+1,j+1);
                    monster->characteristic[j].advancement = t;
                    lua_intexpr(L, &t, "game.monsters[%d].characteristic[%d].mod", i+1,j+1);
                    monster->characteristic[j].mod = t;
                }
            }
        }

        if (lua_intexpr(L, &t, "game.monsters[%d].items.sz", i+1) == 1) {
            int items_sz = t;
            struct itm_item *item;
            for (int j = 0; j < items_sz; j++) {
                if (lua_intexpr(L, &t, "game.monsters[%d].items[%d].uid", i+1,j+1) == 1) {
                    item = itmlst_item_by_uid(t);
                    if (msr_give_item(monster, item) == false) {
                        itm_destroy(item);
                    }
                    if (lua_intexpr(L, &t, "game.monsters[%d].items[%d].position", i+1,j+1) == 1) {
                        enum inv_locations loc = t;
                        inv_move_item_to_location(monster->inventory, item, loc);
                    }
                }
            }
        }
        if (monster->is_player == true) {
            g->player_data.player = monster;
        }
        if (msr_weapons_check(monster) == false) msr_weapon_next_selection(monster);

        msr_insert_monster(monster, map, &monster->pos);
        ai_monster_init(monster, leader_uid);
    }

    if (g->player_data.player == NULL) return false;
    return true;
}

static bool load_map(lua_State *L, struct dm_map **m, int mapid) {
    if (L == NULL) return false;
    uint64_t t;
    int map_sz = 0, x_sz, y_sz, type, threat_min, threat_max, item_chance, monster_chance;
    uint64_t seed;
    if (lua_intexpr(L, &t, "game.maps[%d].map.sz", mapid) == 1) {
        map_sz = t;
        if (lua_intexpr(L, &t, "game.maps[%d].sett.seed", mapid) == 0) return false;
        seed = t;
        if (lua_intexpr(L, &t, "game.maps[%d].sett.type", mapid) == 0) return false;
        type = t;
        if (lua_intexpr(L, &t, "game.maps[%d].sett.size.x", mapid) == 0) return false;
        x_sz = t;
        if (lua_intexpr(L, &t, "game.maps[%d].sett.size.y", mapid) == 0) return false;
        y_sz = t;
        if (lua_intexpr(L, &t, "game.maps[%d].sett.threat_min", mapid) == 0) return false;
        threat_min = t;
        if (lua_intexpr(L, &t, "game.maps[%d].sett.threat_max", mapid) == 0) return false;
        threat_max = t;
        if (lua_intexpr(L, &t, "game.maps[%d].sett.item_chance", mapid) == 0) return false;
        item_chance = t;
        if (lua_intexpr(L, &t, "game.maps[%d].sett.monster_chance", mapid) == 0) return false;
        monster_chance = t;

        struct dm_spawn_settings spwn_sett = {
            .size = cd_create(x_sz,y_sz),
            .threat_lvl_min = threat_min,
            .threat_lvl_max = threat_max,
            .item_chance = item_chance,
            .monster_chance = monster_chance,
            .seed = seed,
            .type = type,
        };

        *m = dm_generate_map(&spwn_sett);
        if (*m != NULL) {
            struct dm_map *map = *m;
            dm_clear_map(map);

            for (int i = 0; i < map_sz; i++) {
                coord_t pos;
                lua_intexpr(L, &t, "game.maps[%d].map[%d].pos.x", mapid, i+1); pos.x = t;
                lua_intexpr(L, &t, "game.maps[%d].map[%d].pos.y", mapid, i+1); pos.y = t;
                if ( (pos.x < map->sett.size.x) && (pos.y < map->sett.size.y) ) {
                    dm_get_map_me(&pos, map)->pos = pos;
                    lua_intexpr(L, &t, "game.maps[%d].map[%d].tile.id", mapid, i+1); dm_get_map_me(&pos, map)->tile = ts_get_tile_specific(t);
                    lua_intexpr(L, &t, "game.maps[%d].map[%d].discovered", mapid, i+1); dm_get_map_me(&pos, map)->discovered = t;

                    if (lua_intexpr(L, &t, "game.maps[%d].map[%d].items.sz", mapid, i+1) == 1) {
                        int items_sz = t;
                        struct itm_item *item;
                        for (int j = items_sz; j > 0; j--) {
                            if (lua_intexpr(L, &t, "game.maps[%d].map[%d].items[%d]", mapid, i+1, j) == 1) {
                                item = itmlst_item_by_uid(t);
                                if (item != NULL) itm_insert_item(item, map, &pos);
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

/**
* This reads the default config file, or the system default if the first is not found.
* When either is read, the application settings are read from file or their defaults are used.
*
* @return returns 0 on succes or 1 on failure.
*/
bool ld_read_save_file(const char *path, struct gm_game *g) {
    if (path == NULL) return false;
    if (g == NULL) return false;

    lg_debug("loading game from %s", path);
    lua_State *L = conf_open(path);

    if (L != NULL) {
        if (load_game(L, g) == false) return false;
        if (load_input(L, g) == false) return false;

        if (options.play_recording == false) {
            if ( (!options.test_mode) && (load_log(L, gbl_log) == false) ) return false;
            if (load_player(L, &g->player_data) == false) return false;
            if (load_items_list(L) == false) return false;
            if (load_map(L, &g->current_map, 1) == false) return false;
            if (load_status_effect_list(L) == false) return false;
            if (load_monsters(L, g->current_map, g) == false) return false;
        }
        lua_close(L);

        return true;
    }
    return false;
}

