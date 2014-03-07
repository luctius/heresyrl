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
#include "monster.h"
#include "items.h"
#include "inventory.h"
#include "dungeon_creator.h"
#include "coord.h"
#include "player.h"
#include "tiles.h"
#include "random.h"

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
static const char* lua_stringexpr(lua_State *L, const char *def, const char* expr)
{
    const char* r = (const char *) def;
    char buf[256] = "";

    /* Assign the Lua expression to a Lua global variable. */
    (void) snprintf(buf, 255, "evalExpr=%s", expr);

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
static int lua_numberexpr(lua_State *L, double *out, const char* format, va_list args)
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
            *out = lua_tonumber(L, -1);
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
    double d = 0;
    int ok = 1;

    va_list args;
    va_start(args, format);
    ok = lua_numberexpr(L, &d, format, args);
    va_end(args);

    if (ok == 1)
    {
        *out = (uint64_t) d;
    }

    return ok;
}

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
        lg_printf_l(LG_DEBUG_LEVEL_ERROR, "load", "Bailing out! No memory.");
        return L;
    }

    luaL_openlibs(L);
    if ( (err = luaL_loadfile(L, file) ) != 0) 
    {
        if (err == LUA_ERRFILE)
        {
            lg_printf_l(LG_DEBUG_LEVEL_INFORMATIONAL, "load", "cannot access config file: %s", file);
        }
        else if (err == LUA_ERRSYNTAX)
        {
            lg_printf_l(LG_DEBUG_LEVEL_ERROR, "load", "syntax error in config file: %s", file);
        }
        else if (err == LUA_ERRERR)
        {
            lg_printf_l(LG_DEBUG_LEVEL_ERROR, "load", "general LUA error when accessing config file: %s", file);
        }
        else if (err == LUA_ERRMEM)
        {
            lg_printf_l(LG_DEBUG_LEVEL_ERROR, "load", "out of memory when accessing config file: %s", file);
        }
        else
        {
            lg_printf_l(LG_DEBUG_LEVEL_ERROR, "load", "Unkown LUA error(%d) when accessing config file: %s", err, file);
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
                lg_printf_l(LG_DEBUG_LEVEL_ERROR, "load", "Run error when executing config file: %s", file);
            }
            else if (err == LUA_ERRERR)
            {
                lg_printf_l(LG_DEBUG_LEVEL_ERROR, "load", "general LUA error when executing config file: %s", file);
            }
            else if (err == LUA_ERRMEM)
            {
                lg_printf_l(LG_DEBUG_LEVEL_ERROR, "load", "out of memory when executing config file: %s", file);
            }
            else
            {
                lg_printf_l(LG_DEBUG_LEVEL_ERROR, "load", "unkown error(%d) when executing config file: %s", err, file);
            }

            lua_close(L);
            L = NULL;
        }
        else lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "load", "loaded config file in memory; parsing now...");
    }

    return L;
}

static bool load_game(lua_State *L, struct gm_game *g) {
    uint64_t t;
    if (L == NULL) return false;

    if (lua_intexpr(L, &t, "game.turn") == 0) return false;
    g->turn = t;

    if (lua_intexpr(L, &t, "game.game_random.seed") == 0) return false;
    g->game_random = random_init_genrand(t);
    if (lua_intexpr(L, &t, "game.game_random.called") == 0) return false;
    random_loop_called(g->game_random, t);

    if (lua_intexpr(L, &t, "game.map_random.seed") == 0) return false;
    g->map_random = random_init_genrand(t);
    if (lua_intexpr(L, &t, "game.map_random.called") == 0) return false;
    random_loop_called(g->map_random, t);

    if (lua_intexpr(L, &t, "game.item_random.seed") == 0) return false;
    g->item_random = random_init_genrand(t);
    if (lua_intexpr(L, &t, "game.item_random.called") == 0) return false;
    random_loop_called(g->item_random, t);

    if (lua_intexpr(L, &t, "game.monster_random.seed") == 0) return false;
    g->monster_random = random_init_genrand(t);
    if (lua_intexpr(L, &t, "game.monster_random.called") == 0) return false;
    random_loop_called(g->monster_random, t);

    if (lua_intexpr(L, &t, "game.ai_random.seed") == 0) return false;
    g->ai_random = random_init_genrand(t);
    if (lua_intexpr(L, &t, "game.ai_random.called") == 0) return false;
    random_loop_called(g->ai_random, t);

    return true;
}

static bool load_player(lua_State *L, struct pl_player *plr) {
    uint64_t t;
    if (L == NULL) return false;

    char *name_ptr;
    if ( (name_ptr = lua_stringexpr(L,"noname", "game.player.name") ) == NULL) return false;
    plr->name = malloc(strlen(name_ptr) );
    strcpy(plr->name,name_ptr);

    if (lua_intexpr(L, &t, "game.player.weapon_selection") == 0) return false;
    plr->weapon_selection = t;
    if (lua_intexpr(L, &t, "game.player.rof_setting_rhand") == 0) return false;
    plr->rof_setting_rhand = t;
    if (lua_intexpr(L, &t, "game.player.rof_setting_lhand") == 0) return false;
    plr->rof_setting_lhand = t;
    return true;
}

static bool load_items_list(lua_State *L) {
    uint64_t t;
    if (L == NULL) return false;
    if (lua_intexpr(L, &t, "game.items.sz") == 0) return false;

    int items_sz = t;
    for (int i = 0; i < items_sz; i++) {
        if (lua_intexpr(L, &t, "game.items[%d].template_id", i+1) == 0) return false;
        int template_id = t;
        struct itm_item *item = itm_create_specific(template_id);
        if (item == NULL) return false;

        lua_intexpr(L, &t, "game.items[%d].uid", i+1); item->uid = t;
        lua_intexpr(L, &t, "game.items[%d].quality", i+1); item->quality = t;
        lua_intexpr(L, &t, "game.items[%d].quantity", i+1); item->stacked_quantity = t;

        switch(item->item_type) {
            case ITEM_TYPE_WEAPON: {
                    struct item_weapon_specific *wpn = &item->specific.weapon;
                    lua_intexpr(L, &t, "game.items[%d].weapon.magazine_left", i+1); wpn->magazine_left = t;
                    lua_intexpr(L, &t, "game.items[%d].weapon.special_quality", i+1); wpn->special_quality = t;
                    lua_intexpr(L, &t, "game.items[%d].weapon.upgrades", i+1); wpn->upgrades = t;
                    lua_intexpr(L, &t, "game.items[%d].weapon.jammed", i+1); wpn->jammed = t;
                } break;
            case ITEM_TYPE_WEARABLE: {
                    /*struct item_wearable_specific *wear = &item->specific.wearable;
                    fprintf(file, "wearable={},");*/
                } break;
            case ITEM_TYPE_FOOD: {
                    struct item_food_specific *food = &item->specific.food;
                    lua_intexpr(L, &t, "game.items[%d].food.nutrition_left", i+1); food->nutrition_left = t;
                } break;
            case ITEM_TYPE_TOOL: {
                    struct item_tool_specific *tool = &item->specific.tool;
                    lua_intexpr(L, &t, "game.items[%d].tool.lit", i+1); tool->lit = t;
                    lua_intexpr(L, &t, "game.items[%d].tool.energy_left", i+1); tool->energy_left = t;
                } break;
            case ITEM_TYPE_AMMO: {
                    struct item_ammo_specific *ammo = &item->specific.ammo;
                    lua_intexpr(L, &t, "game.items[%d].ammo.energy_left", i+1); ammo->energy_left = t;
                } break;
            default: break;
        }
    }
    return true;
}

static bool load_monsters(lua_State *L, struct dc_map *map, struct gm_game *g) {
    uint64_t t;
    if (L == NULL) return false;
    if (lua_intexpr(L, &t, "game.monsters.sz") == 0) return false;

    int monsters_sz = t;
    for (int i = 0; i < monsters_sz; i++) {

        if (lua_intexpr(L, &t, "game.monsters[%d].uid", i+1) == 0) return false;
        struct msr_monster *monster = msr_create(t);
        if (monster == NULL) return false;
        lua_intexpr(L, &t, "game.monsters[%d].template_id", i+1); monster->template_id = t;
        lua_intexpr(L, &t, "game.monsters[%d].race", i+1); monster->race = t;
        lua_intexpr(L, &t, "game.monsters[%d].size", i+1); monster->size = t;
        lua_intexpr(L, &t, "game.monsters[%d].cur_wounds", i+1); monster->cur_wounds = t;
        lua_intexpr(L, &t, "game.monsters[%d].max_wounds", i+1); monster->max_wounds = t;
        lua_intexpr(L, &t, "game.monsters[%d].fatepoints", i+1); monster->fatepoints = t;
        lua_intexpr(L, &t, "game.monsters[%d].race_traits", i+1); monster->race_traits = t;
        lua_intexpr(L, &t, "game.monsters[%d].combat_talents", i+1); monster->combat_talents = t;
        lua_intexpr(L, &t, "game.monsters[%d].career_talents", i+1); monster->career_talents = t;
        lua_intexpr(L, &t, "game.monsters[%d].creature_talents", i+1); monster->creature_talents = t;
        lua_intexpr(L, &t, "game.monsters[%d].is_player", i+1); monster->is_player = t;
        lua_intexpr(L, &t, "game.monsters[%d].pos.x", i+1); monster->pos.x = t;
        lua_intexpr(L, &t, "game.monsters[%d].pos.y", i+1); monster->pos.y = t;

        if (lua_intexpr(L, &t, "game.monsters[%d].skills.sz", i+1) == 1) {
            int skills_sz = t;
            for (int j = 0; j < skills_sz; j++) {
                if (lua_intexpr(L, &t, "game.monsters[%d].skills[%d]", i+1, j+1) == 1) {
                    monster->skills[j] = t;
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
                }
            }
        }

        if (lua_intexpr(L, &t, "game.monsters[%d].items.sz", i+1) == 1) {
            int items_sz = t;
            struct itm_item *item;
            for (int j = 0; j < items_sz; j++) {
                if (lua_intexpr(L, &t, "game.monsters[%d].items[%d].uid", i+1,j+1) == 1) {
                    item = itmlst_item_by_uid(t);
                    inv_add_item(monster->inventory,item);
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
        else msr_insert_monster(monster, map, &monster->pos);
    }
    return true;
}

static bool load_map(lua_State *L, struct dc_map **m, int mapid) {
    if (L == NULL) return false;
    uint64_t t;
    int map_sz = 0, x_sz, y_sz, type, seed;
    if (lua_intexpr(L, &t, "game.maps[%d].map.sz", mapid) == 1) {
        map_sz = t;
        if (lua_intexpr(L, &t, "game.maps[%d].seed", mapid) == 0) return false;
        seed = t;
        if (lua_intexpr(L, &t, "game.maps[%d].type", mapid) == 0) return false;
        type = t;
        if (lua_intexpr(L, &t, "game.maps[%d].size.x", mapid) == 0) return false;
        x_sz = t;
        if (lua_intexpr(L, &t, "game.maps[%d].size.y", mapid) == 0) return false;
        y_sz = t;

        if (x_sz * y_sz != map_sz) return false;

        *m = dc_alloc_map(x_sz, y_sz);
        if (*m != NULL) {
            struct dc_map *map = *m;
            map->seed = seed;
            map->type = type;
            dc_clear_map(map);

            for (int i = 0; i < map_sz; i++) {
                coord_t pos;
                lua_intexpr(L, &t, "game.maps[%d].map[%d].pos.x", mapid, i+1); pos.x = t;
                lua_intexpr(L, &t, "game.maps[%d].map[%d].pos.y", mapid, i+1); pos.y = t;
                if ( (pos.x < map->size.x) && (pos.y < map->size.y) ) {
                    sd_get_map_me(&pos, map)->pos = pos;
                    lua_intexpr(L, &t, "game.maps[%d].map[%d].discovered", mapid, i+1); sd_get_map_me(&pos, map)->discovered = t;
                    lua_intexpr(L, &t, "game.maps[%d].map[%d].tile.id", mapid, i+1); sd_get_map_me(&pos, map)->tile = ts_get_tile_specific(t);

                    if (lua_intexpr(L, &t, "game.maps[%d].map[%d].items.sz", mapid, i+1) == 1) {
                        int items_sz = t;
                        struct itm_item *item;
                        for (int j = items_sz; j > 0; j--) {
                            if (lua_intexpr(L, &t, "game.maps[%d].map[%d].items[%d]", mapid, i+1, j) == 1) {
                                item = itmlst_item_by_uid(t);
                                inv_add_item(sd_get_map_me(&pos,map)->inventory,item);
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
int ld_read_save_file(const char *path, struct gm_game *g) {
    lua_State *L = conf_open(path);
    int errorcode = 1;
    
    if (L != NULL)
    {
        load_player(L, &g->player_data);
        load_items_list(L);
        load_map(L, &g->current_map, 1);
        load_monsters(L, g->current_map, g);
        lua_close(L);
    }
    return errorcode;
}

