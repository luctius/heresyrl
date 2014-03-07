#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"

#include "save.h"
#include "monster.h"
#include "items.h"
#include "inventory.h"
#include "dungeon_creator.h"
#include "coord.h"
#include "player.h"
#include "tiles.h"

static bool sv_save_items(FILE *file, int indent) {
    if (file == NULL) return false;
    int sz = 0;

    fprintf(file, "%*s" "items={\n", indent, ""); { indent += 2;
        struct itm_item *item = NULL;
        while ( (item = itmlst_get_next_item(item) ) != NULL) {
                fprintf(file, "%*s" "{uid=%d,static_id=%d,quality=%d,quantity=%d,",  indent, "", item->uid, item->static_id,item->quality,item->stacked_quantity);
                switch(item->item_type) {
                    case ITEM_TYPE_WEAPON: {
                            struct item_weapon_specific *wpn = &item->specific.weapon;
                            fprintf(file, "weapon={magazine_left=%d,jammed=%d,special_quality=%d,upgrades=%d,}",
                                    wpn->magazine_left,wpn->jammed,wpn->special_quality,wpn->upgrades);
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
    return true;
}

static bool sv_save_map(FILE *file, int indent, struct dc_map *map) {
    if (file == NULL) return false;
    if (map == NULL) return false;
    int sz = 0;

    fprintf(file, "%*s" "{\n", indent, ""); { indent += 2;
        fprintf(file, "%*s" "size={x=%d,y=%d,},\n", indent, "", map->size.x, map->size.y);
        fprintf(file, "%*s" "seed=%d,\n",  indent, "", map->seed);
        fprintf(file, "%*s" "type=%d,\n",  indent, "", map->type);

        fprintf(file, "%*s" "map={\n", indent, ""); { indent += 2;
            coord_t c;
            for (c.x = 0; c.x < map->size.x; c.x++) {
                for (c.y = 0; c.y < map->size.y; c.y++) {
                    struct dc_map_entity *me = sd_get_map_me(&c, map);
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
            fprintf(file, "%*s" "sz=%d,\n",  indent, "", sz);
        } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");
    } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");
    return true;
}

bool sv_save_game(const char *filename, struct gm_game *gm) {
    if (filename == NULL) return false;
    if (gm == NULL) return false;
    int indent = 0;

    FILE *file = fopen(filename, "w");
    fprintf(file, "%*s" "game={\n", indent, ""); { indent += 2;
        fprintf(file, "%*s" "maps={\n", indent, ""); { indent += 2;
            sv_save_map(file, indent, gm->current_map);
        } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");
        sv_save_items(file, indent);
    } indent -= 2; fprintf(file, "%*s" "}\n", indent, "");
    return true;
}
