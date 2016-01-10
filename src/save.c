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

#define __STDC_FORMAT_MACROS 

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

#include "config.h"
#include "save.h"
#include "inventory.h"
#include "coord.h"
#include "input.h"
#include "player.h"
#include "random.h"
#include "options.h"
#include "monster/monster.h"
#include "items/items.h"
#include "dungeon/tiles.h"
#include "dungeon/dungeon_map.h"
#include "careers/careers.h"
#include "status_effects/ground_effects.h"

#define INDENTATION 4
#define svprintf_open(f,fmt) do { fprintf(f,"%*s" fmt "{\n", indent, ""); indent += INDENTATION; } while (0)
#define svprintf_close(f) do { indent -= INDENTATION; fprintf(f,"%*s" "},\n", indent, ""); } while (0)
#define svprintf(f, fmt, a...) fprintf(f,"%*s" fmt "\n", indent, "", ##a)

static bool sv_save_log(FILE *file, int indent, struct logging *lctx) {
    if (file == NULL) return false;

    int log_sz = lg_size(lctx);
    int sz = MAX(log_sz-2, 0);

    int print_ctr = 0;
    if (sz > 0) {
        svprintf_open(file, "log=");

            for (int i = 0; i < lg_size(lctx) -1; i++) {
                struct log_entry *le = lg_peek(lctx, i);
                if ( (le->level == LG_DEBUG_LEVEL_GAME) ||
                     (le->level == LG_DEBUG_LEVEL_GAME_INFO) ) {
                    svprintf_open(file,"");
                        svprintf(file, "turn=%d,",       le->turn);
                        svprintf(file, "repeated=%d,",   le->repeat);
                        svprintf(file, "line=%d,",       le->line);
                        svprintf(file, "level=%d,",      le->level);
                        if (options.debug) svprintf(file, "module=\"%s\",", le->module);
                        svprintf(file, "string=\"%s\",", le->string);
                    svprintf_close(file);
                    print_ctr++;
                }
            }
            svprintf(file, "sz=%d,", print_ctr);
        svprintf_close(file);
        fflush(file);
    }

    return true;
}

static bool sv_save_input(FILE *file, int indent, struct inp_input *input) {
    if (file == NULL) return false;

    svprintf_open(file, "input=");
        svprintf_open(file,"keylog=");
            svprintf(file, "sz=%d,", input->keylog_widx);
            for (int i = 0; i < input->keylog_widx; i++) {
                svprintf(file,"%d,", input->keylog[i]);
            }
        svprintf_close(file);
    svprintf_close(file);

    fflush(file);
    return true;
}

static bool sv_save_player(FILE *file, int indent, struct pl_player *plr) {
    if (file == NULL) return false;

    svprintf_open(file, "player=");
        svprintf(file,"career_id= %d,", plr->career->template_id);
        svprintf(file,"xp_current= %d,", plr->xp_current);
        svprintf(file,"xp_spend= %d,", plr->xp_spend);
    svprintf_close(file);
    fflush(file);
    return true;
}

static bool sv_save_monsters(FILE *file, int indent) {
    if (file == NULL) return false;
    int sz = 0;

    svprintf_open(file, "monsters=");
        struct msr_monster *m = NULL;
        while ( (m = msrlst_get_next_monster(m) ) != NULL) {
            if (m->dead) continue;

            svprintf_open(file, "");
                svprintf(file,"uid=%d,", m->uid);
                svprintf(file,"template_id=%d,", m->template_id);
                svprintf(file,"race=%d,", m->race);
                svprintf(file,"size=%d,", m->size);
                svprintf(file,"gender=%d,", m->gender);
                svprintf(file,"fate_points=%d,", m->fate_points);
                svprintf(file,"insanity_points=%d,", m->insanity_points);
                svprintf(file,"corruption_points=%d,", m->corruption_points);
                svprintf(file,"is_player=%d,", m->is_player);
                svprintf(file,"wpn_sel=%d,", m->wpn_sel);
                svprintf(file,"ai_leader=%u,", msr_get_ai_ctx(m)->leader_uid );
                svprintf(file,"pos={x=%d,y=%d,},", m->pos.x,m->pos.y);
                svprintf(file,"creature_traits=%"PRIu64",",m->creature_traits);
                if (m->unique_name != NULL) svprintf(file, "unique_name=\"%s\",",m->unique_name);

                svprintf_open(file,"wounds=");
                    svprintf(file,"curr=%d,", m->wounds.curr);
                    svprintf(file,"max=%d,", m->wounds.max);
                    svprintf(file,"added=%d,", m->wounds.added);
                svprintf_close(file);

                svprintf(file,"idle_counter=%d,", m->idle_counter);

                svprintf_open(file,"evasion=");
                    for (int i = 0; i < MSR_EVASION_MAX; i++) {
                        svprintf(file,"%" PRIu32 ",", m->evasion_last_used[i]);
                    }
                    svprintf(file,"sz=%d,", MSR_EVASION_MAX);
                svprintf_close(file);

                svprintf_open(file,"talents=");
                    int t_sz = 0;
                    for (int i = 0; i < MSR_NR_TALENTS_MAX; i++) {
                        if (m->talents[i] == TLT_NONE) break;

                        svprintf(file,"%" PRIu8 ",", m->talents[i]);
                        t_sz++;
                    }
                    svprintf(file,"sz=%d,", t_sz);
                svprintf_close(file);

                svprintf_open(file,"status_effects=");
                    int se_sz = 0;
                    svprintf(file,"sz=%d,", se_list_size(m->status_effects) );
                    struct status_effect *c = NULL;
                    while ( (c = se_list_get_next_status_effect(m->status_effects, c) ) != NULL) {
                        svprintf(file,"%d,", c->uid);
                        se_sz++;
                    }
                svprintf_close(file);

                svprintf_open(file,"skills=");
                    svprintf(file,"sz=%d,", MSR_SKILL_RATE_MAX);
                    for (int i = 0; i < MSR_SKILL_RATE_MAX; i++) {
                        svprintf(file,"%"PRIu32",", m->skills[i]);
                    }
                svprintf_close(file);

                svprintf_open(file,"characteristic=");
                    svprintf(file,"sz=%d,", MSR_CHAR_MAX);
                    for (int i = 0; i < MSR_CHAR_MAX; i++) {
                        svprintf_open(file,"");
                            svprintf(file,"base_value=%d,",  m->characteristic[i].base_value);
                            svprintf(file,"advancement=%d,", m->characteristic[i].advancement);
                            svprintf(file,"mod=%d,",         m->characteristic[i].mod);
                        svprintf_close(file);
                    }
                svprintf_close(file);

                int invsz = 0;
                if ( (invsz = inv_inventory_size(m->inventory) ) > 0) {
                    svprintf_open(file,"items=");
                        struct itm_item *item = NULL;
                        while ( (item = inv_get_next_item(m->inventory, item)) != NULL) {
                            if (item->dropable == false) {
                                invsz -= 1;
                                continue;
                            }

                            svprintf_open(file,"");
                                svprintf(file, "uid=%d,", item->uid);
                                svprintf(file, "position=%"PRIu32",", inv_get_item_locations(m->inventory, item) );
                            svprintf_close(file);
                        }
                        svprintf(file, "sz=%d,", invsz);
                    svprintf_close(file);
                }
            svprintf_close(file);

            fflush(file);
            sz++;
        }
    svprintf_close(file);

    fflush(file);
    return true;
}

static bool sv_save_status_effects(FILE *file, int indent) {
    if (file == NULL) return false;
    int sz = 0;

    svprintf_open(file, "status_effects=");
        struct status_effect *se = NULL;
        while ( (se = selst_get_next_status_effect(se) ) != NULL) {
            svprintf_open(file, "");
                svprintf(file, "uid=%d,",  se->uid);
                svprintf(file, "tid=%d,",  se->template_id);

                svprintf(file,"duration_energy_min=%d,", se->duration_energy_min);
                svprintf(file,"duration_energy_max=%d,", se->duration_energy_max);
                svprintf(file,"duration_energy=%d,",     se->duration_energy);

                int e_sz = 0;
                svprintf_open(file, "effects=");
                    for (int i = 0; i < STATUS_EFFECT_MAX_NR_EFFECTS; i++) {
                        if (se->effects[i].effect == EF_NONE) break;
                        if (status_effect_has_flag(se, SEF_ACTIVE) == false) break;
                        svprintf_open(file, "");
                            svprintf(file,"effect=%d,",               se->effects[i].effect);
                            svprintf(file,"effect_setting_flags=%d,", se->effects[i].effect_setting_flags);
                            svprintf(file,"tick_interval_energy=%d,", se->effects[i].tick_interval_energy);
                            svprintf(file,"tick_energy=%d,",          se->effects[i].tick_energy);
                            svprintf(file,"ticks_applied=%d,",        se->effects[i].ticks_applied);
                            svprintf(file,"param=%d,",                se->effects[i].param);
                        svprintf_close(file);
                        e_sz++;
                    }
                    svprintf(file,"sz=%d,", e_sz);
                svprintf_close(file);
            svprintf_close(file);
            sz++;
        }

        svprintf(file, "sz=%d,\n", sz);
    svprintf_close(file);

    sz = 0;
    svprintf_open(file, "ground_effects=");
        struct ground_effect *ge = NULL;
        while ( (ge = gelst_get_next(ge) ) != NULL) {
            svprintf_open(file, "");
                svprintf(file, "uid=%d,",           ge->uid);
                svprintf(file, "tid=%d,",           ge->tid);

                svprintf(file,"min_energy=%d,",     ge->min_energy);
                svprintf(file,"max_energy=%d,",     ge->max_energy);
                svprintf(file,"current_energy=%d,", ge->current_energy);
                svprintf(file,"se_id=%d,",          ge->se_id);
                svprintf_open(file, "pos=");
                    svprintf(file,"x=%d,",  ge->me->pos.x);
                    svprintf(file,"y=%d,",  ge->me->pos.y);
                svprintf_close(file);
            svprintf_close(file);
            sz++;
        }

        fprintf(file, "%*s" "sz=%d,\n",  indent, "", sz);
    svprintf_close(file);

    fflush(file);
    return true;
}

static bool sv_save_items(FILE *file, int indent) {
    if (file == NULL) return false;
    int sz = 0;

    svprintf_open(file, "items=");
        struct itm_item *item = NULL;
        while ( (item = itmlst_get_next_item(item) ) != NULL) {
            if (item->dropable == false) continue;

            svprintf_open(file, "");
                svprintf(file, "uid=%d,",  item->uid);
                svprintf(file, "template_id=%d,",  item->template_id);
                svprintf(file, "quality=%d,",      item->quality);
                svprintf(file, "quantity=%d,",     item->stacked_quantity);
                svprintf(file, "identified=%d,",   item->identified);
                switch(item->item_type) {
                    case ITEM_TYPE_WEAPON: {
                            struct item_weapon_specific *wpn = &item->specific.weapon;
                            svprintf_open(file, "weapon=");
                                svprintf(file, "magazine_left=%d,", wpn->magazine_left);
                                svprintf(file, "ammo_used_template_id=%d,", wpn->ammo_used_template_id);
                                svprintf(file, "jammed=%d,", wpn->jammed);
                                svprintf(file, "special_quality=%"PRIu64",", wpn->special_quality);
                                svprintf(file, "upgrades=%"PRIu64",", wpn->upgrades);
                                svprintf(file, "rof_set=%d,", wpn->rof_set);
                            svprintf_close(file);
                        } break;
                    case ITEM_TYPE_WEARABLE: {
                            /*struct item_wearable_specific *wear = &item->specific.wearable;
                            svprintf(file, "wearable={},");*/
                        } break;
                    case ITEM_TYPE_FOOD: {
                            struct item_food_specific *food = &item->specific.food;
                            svprintf_open(file, "food=");
                                svprintf(file, "nutrition_left=%d,", food->nutrition_left);
                                svprintf(file, "side_effect_chance=%d,", food->side_effect_chance);
                                svprintf(file, "side_effect=%d,", food->side_effect);
                            svprintf_close(file);
                        } break;
                    case ITEM_TYPE_TOOL: {
                            struct item_tool_specific *tool = &item->specific.tool;
                            svprintf_open(file, "tool=");
                                svprintf(file, "lit=%d,", tool->lit);
                                svprintf(file, "energy=%d,", tool->energy);
                            svprintf_close(file);
                        } break;
                    case ITEM_TYPE_AMMO: { } break;
                    default: break;
                }
            svprintf_close(file);
            sz++;
        }
        svprintf(file, "sz=%d,", sz);
    svprintf_close(file);
    fflush(file);
    return true;
}

static bool sv_save_map(FILE *file, int indent, struct dm_map *map) {
    if (file == NULL) return false;
    if (dm_verify_map(map) == false) return false;
    int sz = 0;

    svprintf_open(file, "");
        svprintf_open(file, "size=");
            svprintf(file, "x=%d,", map->size.x);
            svprintf(file, "y=%d,", map->size.y);
        svprintf_close(file);
        svprintf(file, "seed=%lu,",  map->seed);
        svprintf(file, "type=%d,",  map->type);
        svprintf(file, "threat_lvl=%d,", map->threat_lvl);

        svprintf_open(file, "map=");
            coord_t c;
            for (c.x = 0; c.x < map->size.x; c.x++) {
                for (c.y = 0; c.y < map->size.y; c.y++) {
                    struct dm_map_entity *me = dm_get_map_me(&c, map);
                    if (me->discovered == true || inv_inventory_size(me->inventory) > 0) {
                        svprintf_open(file, "");
                            svprintf_open(file, "pos=");
                                svprintf(file, "x=%d,", me->pos.x);
                                svprintf(file, "y=%d,", me->pos.y);
                            svprintf_close(file);
                            svprintf(file, "discovered=%d,", me->discovered);
                            svprintf_open(file, "tile=");
                                svprintf(file, "id=%d,",  me->tile->id);
                            svprintf_close(file);

                        int invsz = 0;
                        if ( (invsz = inv_inventory_size(me->inventory) ) > 0) {
                            svprintf_open(file, "items=");
                            struct itm_item *item = NULL;
                            while ( (item = inv_get_next_item(me->inventory, item)) != NULL) {
                                if (item->dropable == false) {
                                    invsz -= 1;
                                    continue;
                                }

                                svprintf(file, "%d,", item->uid);
                            }
                            svprintf(file, "sz=%d,", invsz);
                            svprintf_close(file);
                        }

                        svprintf_close(file);
                        sz++;
                    }
                }
            }
            fprintf(file, "%*s" "sz=%d,\n",  indent, "", sz);
        svprintf_close(file);
    svprintf_close(file);
    fflush(file);
    return true;
}

bool sv_save_game(const char *filename, struct gm_game *gm) {
    if (filename == NULL) return false;
    if (gm == NULL) return false;
    int indent = 0;

    lg_debug("saving game to %s", filename);
    FILE *file = fopen(filename, "w");
    svprintf_open(file, "game=");
        svprintf(file, "version=\"%s\",", VERSION);
        svprintf(file, "git_version=\"%s\",", GIT_VERSION);
        svprintf(file, "turn=%" PRIi64 ",", gm->turn);
        svprintf_open(file, "random=");
            svprintf(file, "seed=%lu,", random_get_seed(gm->random) );
            svprintf(file, "called=%d,", random_get_nr_called(gm->random) );
        svprintf_close(file);

        sv_save_player(file, indent, &gm->player_data);

        sv_save_items(file, indent);
        sv_save_status_effects(file, indent);
        sv_save_monsters(file, indent);

        svprintf_open(file, "maps=");
            sv_save_map(file, indent, gm->current_map);
        svprintf_close(file);

        sv_save_input(file, indent, gm->input);
        sv_save_log(file, indent, gbl_log);

    /* special case, no comma at the end */
    indent -= INDENTATION; svprintf(file, "}\n");
    fflush(file);
    fclose(file);

    return true;
}
