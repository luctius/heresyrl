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
#include "monster/monster.h"
#include "items/items.h"
#include "dungeon/tiles.h"
#include "dungeon/dungeon_map.h"
#include "careers/careers.h"
#include "status_effects/ground_effects.h"

static bool sv_save_log(FILE *file, int indent, struct logging *lctx) {
    if (file == NULL) return false;

    int log_sz = lg_size(lctx);
    int sz = MAX(log_sz-2, 0);

    int print_ctr = 0;
    if (sz > 0) {
        fprintf(file, "%*s" "log={\n", indent, ""); { indent += 2;

            for (int i = 0; i < lg_size(lctx) -1; i++) {
                struct log_entry *le = lg_peek(lctx, i);
                if ( (le->level == LG_DEBUG_LEVEL_GAME) ||
                     (le->level == LG_DEBUG_LEVEL_GAME_INFO) ) {
                    fprintf(file,"%*s" "{", indent, "");
                    fprintf(file, "turn=%d,",       le->turn);
                    fprintf(file, "repeated=%d,",   le->repeat);
                    fprintf(file, "line=%d,",       le->line);
                    fprintf(file, "level=%d,",      le->level);
                    fprintf(file, "module=\"%s\",", le->module);
                    fprintf(file, "string=\"%s\",", le->string);
                    fprintf(file, "},\n");
                    print_ctr++;
                }
            }
            fprintf(file, "%*s" "sz=%d,\n", indent, "", print_ctr);
        } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");
        fflush(file);
    }

    return true;
}

static bool sv_save_input(FILE *file, int indent, struct inp_input *input) {
    if (file == NULL) return false;

    fprintf(file, "%*s" "input={", indent, ""); { indent += 2;

        fprintf(file,"keylog={sz=%d,", input->keylog_widx);
        for (int i = 0; i < input->keylog_widx; i++) {
            fprintf(file,"%d,", input->keylog[i]);
        }
        fprintf(file, "},");

    } indent -= 2; fprintf(file, "},\n");

    fflush(file);
    return true;
}

static bool sv_save_player(FILE *file, int indent, struct pl_player *plr) {
    if (file == NULL) return false;

    fprintf(file, "%*s" "player={", indent, "");
    fprintf(file,"career_id  = %d,", plr->career->template_id);
    fprintf(file,"xp_current = %d,", plr->xp_current);
    fprintf(file,"xp_spend   = %d,", plr->xp_spend);
    fprintf(file, "},\n");
    fflush(file);
    return true;
}

static bool sv_save_monsters(FILE *file, int indent) {
    if (file == NULL) return false;
    int sz = 0;

    fprintf(file, "%*s" "monsters={\n", indent, ""); { indent += 2;
        struct msr_monster *m = NULL;
        while ( (m = msrlst_get_next_monster(m) ) != NULL) {
            if (m->dead) continue;

            fprintf(file,"%*s" "{uid=%d,", indent, "", m->uid);
            fprintf(file,"template_id=%d,", m->template_id);
            fprintf(file,"race=%d,", m->race);
            fprintf(file,"size=%d,", m->size);
            fprintf(file,"gender=%d,", m->gender);
            fprintf(file,"cur_wounds=%d,", m->cur_wounds);
            fprintf(file,"max_wounds=%d,", m->max_wounds);
            fprintf(file,"wounds_added=%d,", m->wounds_added);
            fprintf(file,"fate_points=%d,", m->fate_points);
            fprintf(file,"insanity_points=%d,", m->insanity_points);
            fprintf(file,"corruption_points=%d,", m->corruption_points);
            fprintf(file,"is_player=%d,", m->is_player);
            fprintf(file,"wpn_sel=%d,", m->wpn_sel);
            fprintf(file,"ai_leader=%u,", msr_get_ai_ctx(m)->leader_uid );
            fprintf(file,"pos={x=%d,y=%d,},", m->pos.x,m->pos.y);
            fprintf(file,"creature_traits=%"PRIu64",",m->creature_traits);
            if (m->unique_name != NULL) fprintf(file, "unique_name=\"%s\",",m->unique_name);

            fprintf(file,"idle_counter=%d,", m->idle_counter);

            fprintf(file,"evasion={");
            for (int i = 0; i < MSR_EVASION_MAX; i++) {
                fprintf(file,"%" PRIu32 ",", m->evasion_last_used[i]);
            }
            fprintf(file,"sz=%d,},", MSR_EVASION_MAX);

            fprintf(file,"talents={");
            int t_sz = 0;
            for (int i = 0; i < MSR_NR_TALENTS_MAX; i++) {
                if (m->talents[i] == TLT_NONE) break;

                fprintf(file,"%" PRIu8 ",", m->talents[i]);
                t_sz++;
            }
            fprintf(file,"sz=%d,},", t_sz);

            int se_sz = 0;
            fprintf(file,"status_effects={sz=%d,", se_list_size(m->status_effects) );
            struct status_effect *c = NULL;
            while ( (c = se_list_get_next_status_effect(m->status_effects, c) ) != NULL) {
                fprintf(file,"%d,", c->uid);
                se_sz++;
            }
            fprintf(file,"},");

            fprintf(file,"skills={sz=%d,", MSR_SKILL_RATE_MAX);
            for (int i = 0; i < MSR_SKILL_RATE_MAX; i++) {
                fprintf(file,"%"PRIu32",", m->skills[i]);
            }
            fprintf(file, "},");

            fprintf(file,"characteristic={sz=%d,", MSR_CHAR_MAX);
            for (int i = 0; i < MSR_CHAR_MAX; i++) {
                fprintf(file,"{");
                    fprintf(file,"base_value=%d,",  m->characteristic[i].base_value);
                    fprintf(file,"advancement=%d,", m->characteristic[i].advancement);
                    fprintf(file,"mod=%d,",         m->characteristic[i].mod);
                fprintf(file,"},");
            }
            fprintf(file, "},");

            int invsz = 0;
            if ( (invsz = inv_inventory_size(m->inventory) ) > 0) {
                fprintf(file, "items={");
                struct itm_item *item = NULL;
                while ( (item = inv_get_next_item(m->inventory, item)) != NULL) {
                    if (item->dropable == false) {
                        invsz -= 1;
                        continue;
                    }

                    fprintf(file, "{uid=%d,position=%"PRIu32"},", item->uid, inv_get_item_locations(m->inventory, item) );
                }
                fprintf(file, "sz=%d,", invsz);
                fprintf(file, "},");
            }
            fprintf(file, "},\n");

            fflush(file);
            sz++;
        }
        fprintf(file, "%*s" "sz=%d,\n", indent, "", sz);
    } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");

    fflush(file);
    return true;
}

static bool sv_save_status_effects(FILE *file, int indent) {
    if (file == NULL) return false;
    int sz = 0;

    fprintf(file, "%*s" "status_effects={\n", indent, ""); { indent += 2;
        struct status_effect *se = NULL;
        while ( (se = selst_get_next_status_effect(se) ) != NULL) {
            fprintf(file, "%*s" "{uid=%d,",  indent, "", se->uid);
            fprintf(file, "tid=%d,",  se->template_id);

            fprintf(file,"duration_energy_min=%d,", se->duration_energy_min);
            fprintf(file,"duration_energy_max=%d,", se->duration_energy_max);
            fprintf(file,"duration_energy=%d,",     se->duration_energy);

            int e_sz = 0;
            fprintf(file,"effects={");
            for (int i = 0; i < STATUS_EFFECT_MAX_NR_EFFECTS; i++) {
                if (se->effects[i].effect == EF_NONE) i = STATUS_EFFECT_MAX_NR_EFFECTS;
                    fprintf(file,"{");
                        fprintf(file,"effect=%d,",               se->effects[i].effect);
                        fprintf(file,"effect_setting_flags=%d,", se->effects[i].effect_setting_flags);
                        fprintf(file,"tick_interval_energy=%d,", se->effects[i].tick_interval_energy);
                        fprintf(file,"tick_energy=%d,",          se->effects[i].tick_energy);
                        fprintf(file,"ticks_applied=%d,",        se->effects[i].ticks_applied);
                        fprintf(file,"param=%d,",                se->effects[i].param);
                    fprintf(file,"},");
                    e_sz++;
                }
                fprintf(file,"sz=%d,},", e_sz);
            fprintf(file, "},\n");
            sz++;
        }

        fprintf(file, "%*s" "sz=%d,\n",  indent, "", sz);
    } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");

    sz = 0;
    fprintf(file, "%*s" "ground_effects={\n", indent, ""); { indent += 2;
        struct ground_effect *ge = NULL;
        while ( (ge = gelst_get_next(ge) ) != NULL) {
            //if (ge->me->discovered == true) 
            {
                fprintf(file, "%*s" "{uid=%d,",  indent, "", ge->uid);
                fprintf(file, "tid=%d,",                     ge->tid);

                fprintf(file,"min_energy=%d,",      ge->min_energy);
                fprintf(file,"max_energy=%d,",      ge->max_energy);
                fprintf(file,"current_energy=%d,",  ge->current_energy);
                fprintf(file,"se_id=%d,",           ge->se_id);
                fprintf(file,"pos={x=%d,y=%d,},",   ge->me->pos.x,ge->me->pos.y);

                fprintf(file, "},\n");
                sz++;
            }
        }

        fprintf(file, "%*s" "sz=%d,\n",  indent, "", sz);
    } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");

    fflush(file);
    return true;
}

static bool sv_save_items(FILE *file, int indent) {
    if (file == NULL) return false;
    int sz = 0;

    fprintf(file, "%*s" "items={\n", indent, ""); { indent += 2;
        struct itm_item *item = NULL;
        while ( (item = itmlst_get_next_item(item) ) != NULL) {
            if (item->dropable == false) continue;

                fprintf(file, "%*s" "{uid=%d,",  indent, "", item->uid);
                fprintf(file, "template_id=%d,",  item->template_id);
                fprintf(file, "quality=%d,",      item->quality);
                fprintf(file, "quantity=%d,",     item->stacked_quantity);
                switch(item->item_type) {
                    case ITEM_TYPE_WEAPON: {
                            struct item_weapon_specific *wpn = &item->specific.weapon;
                            fprintf(file, "weapon={magazine_left=%d,ammo_used_template_id=%d,jammed=%d,special_quality=%"PRIu64",upgrades=%"PRIu64",rof_set=%d},",
                                    wpn->magazine_left,wpn->ammo_used_template_id,wpn->jammed,wpn->special_quality,wpn->upgrades,wpn->rof_set);
                        } break;
                    case ITEM_TYPE_WEARABLE: {
                            /*struct item_wearable_specific *wear = &item->specific.wearable;
                            fprintf(file, "wearable={},");*/
                        } break;
                    case ITEM_TYPE_FOOD: {
                            struct item_food_specific *food = &item->specific.food;
                            fprintf(file, "food={nutrition_left=%d},", food->nutrition_left);
                        } break;
                    case ITEM_TYPE_TOOL: {
                            struct item_tool_specific *tool = &item->specific.tool;
                            fprintf(file, "tool={lit=%d,energy_left=%d},", tool->lit, tool->energy_left);
                        } break;
                    case ITEM_TYPE_AMMO: { } break;
                    default: break;
                }
            fprintf(file, "},\n");
            sz++;
        }
        fprintf(file, "%*s" "sz=%d,\n",  indent, "", sz);
    } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");
    fflush(file);
    return true;
}

static bool sv_save_map(FILE *file, int indent, struct dm_map *map) {
    if (file == NULL) return false;
    if (dm_verify_map(map) == false) return false;
    int sz = 0;

    fprintf(file, "%*s" "{\n", indent, ""); { indent += 2;
        fprintf(file, "%*s" "size={x=%d,y=%d,},\n", indent, "", map->size.x, map->size.y);
        fprintf(file, "%*s" "seed=%lu,\n",  indent, "", map->seed);
        fprintf(file, "%*s" "type=%d,\n",  indent, "", map->type);
        fprintf(file, "%*s" "threat_lvl=%d,\n",  indent, "", map->threat_lvl);

        fprintf(file, "%*s" "map={\n", indent, ""); { indent += 2;
            coord_t c;
            for (c.x = 0; c.x < map->size.x; c.x++) {
                for (c.y = 0; c.y < map->size.y; c.y++) {
                    struct dm_map_entity *me = dm_get_map_me(&c, map);
                    if (me->discovered == true || inv_inventory_size(me->inventory) > 0) {
                        fprintf(file, "%*s" "{pos={x=%d,y=%d,},discovered=%d,tile={id=%d,},",  indent, "", me->pos.x, me->pos.y, me->discovered, me->tile->id);

                        int invsz = 0;
                        if ( (invsz = inv_inventory_size(me->inventory) ) > 0) {
                            fprintf(file, "items={");
                            struct itm_item *item = NULL;
                            while ( (item = inv_get_next_item(me->inventory, item)) != NULL) {
                                if (item->dropable == false) {
                                    invsz -= 1;
                                    continue;
                                }

                                fprintf(file, "%d,", item->uid);
                            }
                            fprintf(file, "sz=%d,", invsz);
                            fprintf(file, "},");
                        }
                        
                        fprintf(file, "},\n");
                        sz++;
                    }
                }
            }
            fprintf(file, "%*s" "sz=%d,\n",  indent, "", sz);
        } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");
    } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");
    fflush(file);
    return true;
}

bool sv_save_game(const char *filename, struct gm_game *gm) {
    if (filename == NULL) return false;
    if (gm == NULL) return false;
    int indent = 0;

    lg_debug("saving game to %s", filename);
    FILE *file = fopen(filename, "w");
    fprintf(file, "%*s" "game={\n", indent, ""); { indent += 2;
        fprintf(file, "%*s" "version=\"%s\",\n", indent, "", VERSION);
        fprintf(file, "%*s" "git_version=\"%s\",\n", indent, "", GIT_VERSION);
        fprintf(file, "%*s" "turn=%" PRIi64 ",\n", indent, "", gm->turn);
        fprintf(file, "%*s" "random={seed=%lu,called=%d},\n", indent, "", 
                random_get_seed(gm->random), random_get_nr_called(gm->random) );

        sv_save_player(file, indent, &gm->player_data);

        sv_save_items(file, indent);
        sv_save_status_effects(file, indent);
        sv_save_monsters(file, indent);

        fprintf(file, "%*s" "maps={\n", indent, ""); { indent += 2;
            sv_save_map(file, indent, gm->current_map);
        } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");

        sv_save_input(file, indent, gm->input);
        sv_save_log(file, indent, gbl_log);
    } indent -= 2; fprintf(file, "%*s" "}\n", indent, "");
    fflush(file);
    fclose(file);

    return true;
}
