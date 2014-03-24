#define __STDC_FORMAT_MACROS 

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

#include "save.h"
#include "monster.h"
#include "items.h"
#include "inventory.h"
#include "dungeon_creator.h"
#include "coord.h"
#include "player.h"
#include "tiles.h"
#include "random.h"

static bool sv_save_player(FILE *file, int indent, struct pl_player *plr) {
    if (file == NULL) return false;

    fprintf(file, "%*s" "player={", indent, "");
    fprintf(file, "name=\"%s\",},\n", plr->name);
    fflush(file);
    return true;
}

static bool sv_save_monsters(FILE *file, int indent) {
    if (file == NULL) return false;
    int sz = 0;

    fprintf(file, "%*s" "monsters={\n", indent, ""); { indent += 2;
        struct msr_monster *m = NULL;
        while ( (m = msrlst_get_next_monster(m) ) != NULL) {
            fprintf(file, "%*s" "{uid=%d,template_id=%d,race=%d,size=%d,gender=%d,cur_wounds=%d,max_wounds=%d,",  indent, "", 
                    m->uid, m->template_id,m->race,m->size,m->gender,m->cur_wounds,m->max_wounds);
            fprintf(file,"fatepoints=%d,is_player=%d,wpn_sel=%d,pos={x=%d,y=%d,},", m->fatepoints,m->is_player,m->wpn_sel,m->pos.x,m->pos.y);
            if (m->unique_name != NULL) fprintf(file, "uname=\"%s\",creature_traits=%"PRIu64",",m->unique_name, m->creature_traits);

            int tmax = ((TALENTS_MAX >> TALENTS_IDX_OFFSET) & 0x0F)+1;
            fprintf(file,"talents={sz=%d,", tmax);
            for (int i = 0; i < tmax; i++) {
                fprintf(file,"%" PRIu64 ",", m->talents[i]);
            }
            fprintf(file, "},");

            fprintf(file,"skills={sz=%d,", MSR_SKILL_RATE_MAX);
            for (int i = 0; i < MSR_SKILL_RATE_MAX; i++) {
                fprintf(file,"%"PRIu64",", m->skills[i]);
            }
            fprintf(file, "},");

            fprintf(file,"characteristic={sz=%d,", MSR_CHAR_MAX);
            for (int i = 0; i < MSR_CHAR_MAX; i++) {
                fprintf(file,"{base_value=%d,advancement=%d},", m->characteristic[i].base_value, m->characteristic[i].advancement);
            }
            fprintf(file, "},");

            int invsz = 0;
            if ( (invsz = inv_inventory_size(m->inventory) ) > 0) {
                fprintf(file, "items={");
                struct itm_item *item = NULL;
                for (int i = 0; i < invsz; i++) {
                    item = inv_get_next_item(m->inventory, item);
                    if (item != NULL) fprintf(file, "{uid=%d,position=%"PRIu64"},", item->uid, inv_get_item_locations(m->inventory, item) );
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

static bool sv_save_items(FILE *file, int indent) {
    if (file == NULL) return false;
    int sz = 0;

    fprintf(file, "%*s" "items={\n", indent, ""); { indent += 2;
        struct itm_item *item = NULL;
        while ( (item = itmlst_get_next_item(item) ) != NULL) {
                fprintf(file, "%*s" "{uid=%d,template_id=%d,quality=%d,quantity=%d,",  indent, "", item->uid, item->template_id,item->quality,item->stacked_quantity);
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
                    case ITEM_TYPE_AMMO: {
                            struct item_ammo_specific *ammo = &item->specific.ammo;
                            fprintf(file, "ammo={energy_left=%d},", ammo->energy_left);
                        } break;
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

static bool sv_save_map(FILE *file, int indent, struct dc_map *map) {
    if (file == NULL) return false;
    if (dc_verify_map(map) == false) return false;
    int sz = 0;

    fprintf(file, "%*s" "{\n", indent, ""); { indent += 2;
        fprintf(file, "%*s" "size={x=%d,y=%d,},\n", indent, "", map->size.x, map->size.y);
        fprintf(file, "%*s" "seed=%d,\n",  indent, "", map->seed);
        fprintf(file, "%*s" "type=%d,\n",  indent, "", map->type);
        fprintf(file, "%*s" "threat_lvl=%d,\n",  indent, "", map->threat_lvl);

        fprintf(file, "%*s" "map={\n", indent, ""); { indent += 2;
            coord_t c;
            for (c.x = 0; c.x < map->size.x; c.x++) {
                for (c.y = 0; c.y < map->size.y; c.y++) {
                    struct dc_map_entity *me = sd_get_map_me(&c, map);
                    if (me->discovered == true || inv_inventory_size(me->inventory) > 0) {
                        fprintf(file, "%*s" "{pos={x=%d,y=%d,},discovered=%d,tile={id=%d,},",  indent, "", me->pos.x, me->pos.y, me->discovered, me->tile->id);
                        int invsz = 0;
                        if ( (invsz = inv_inventory_size(me->inventory) ) > 0) {
                            fprintf(file, "items={");
                            struct itm_item *item = NULL;
                            for (int i = 0; i < invsz; i++) {
                                item = inv_get_next_item(me->inventory, item);
                                if (item != NULL) fprintf(file, "%d,", item->uid);
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

    FILE *file = fopen(filename, "w");
    fprintf(file, "%*s" "game={\n", indent, ""); { indent += 2;
        fprintf(file, "%*s" "turn=%d,\n", indent, "", gm->turn);
        fprintf(file, "%*s" "game_random={seed=%d,called=%d},\n", indent, "", 
                random_get_seed(gm->game_random), random_get_nr_called(gm->game_random) );
        fprintf(file, "%*s" "map_random={seed=%d,called=%d},\n", indent, "", 
                random_get_seed(gm->map_random), random_get_nr_called(gm->map_random) );
        fprintf(file, "%*s" "spawn_random={seed=%d,called=%d},\n", indent, "", 
                random_get_seed(gm->spawn_random), random_get_nr_called(gm->spawn_random) );
        fprintf(file, "%*s" "ai_random={seed=%d,called=%d},\n", indent, "", 
                random_get_seed(gm->ai_random), random_get_nr_called(gm->ai_random) );

        sv_save_player(file, indent, &gm->player_data);
        sv_save_items(file, indent);
        sv_save_monsters(file, indent);
        fprintf(file, "%*s" "maps={\n", indent, ""); { indent += 2;
            sv_save_map(file, indent, gm->current_map);
        } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");
    } indent -= 2; fprintf(file, "%*s" "}\n", indent, "");
    fflush(file);
    return true;
}
