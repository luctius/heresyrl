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

    fprintf(file, "%*s" "items={\n", indent, ""); { indent += 2;
        struct itm_item *item = NULL;
        while ( (item = itmlst_get_next_item(item) ) != NULL) {
                fprintf(file, "%*s" "{id=%d,list_id=%d",  indent, "", item->save_id, item->list_id);
            fprintf(file, "},\n");
        }
    } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");
    return true;
}

static bool sv_save_map(FILE *file, int indent, struct dc_map *map) {
    if (file == NULL) return false;
    if (map == NULL) return false;

    fprintf(file, "%*s" "size={x=%d,y=%d,},\n", indent, "", map->size.x, map->size.y);
    fprintf(file, "%*s" "seed=%d,\n",  indent, "", map->seed);
    fprintf(file, "%*s" "type=%d,\n",  indent, "", map->type);

    fprintf(file, "%*s" "map={\n", indent, ""); { indent += 2;
        coord_t c;
        for (c.x = 0; c.x < map->size.x; c.x++) {
            for (c.y = 0; c.y < map->size.y; c.y++) {
                struct dc_map_entity *me = sd_get_map_me(&c, map);
                fprintf(file, "%*s" "{pos={x=%d,y=%d,},discovered=%d,tile={type=%d,},",  indent, "", me->pos.x, me->pos.y, me->discovered, me->tile->type);
                int invsz = 0;
                if ( (invsz = inv_inventory_size(me->inventory) ) != 0) {
                    fprintf(file, "items={");
                    struct itm_item *item = NULL;
                    for (int i = 0; i < invsz; i++) {
                        item = inv_get_next_item(me->inventory, item);
                        if (item != NULL) fprintf(file, "%d,", item->save_id);
                    }
                    fprintf(file, "},");
                }
                fprintf(file, "},\n");
            }
        }
    } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");
    return true;
}

bool sv_save_game(const char *filename, struct gm_game *gm) {
    if (filename == NULL) return false;
    if (gm == NULL) return false;
    int indent = 0;

    itmlst_truncate_ids();

    FILE *file = fopen(filename, "w");
    fprintf(file, "%*s" "game={\n", indent, ""); { indent += 2;
        fprintf(file, "%*s" "current_map={\n", indent, ""); { indent += 2;
            sv_save_map(file, indent, gm->current_map);
        } indent -= 2; fprintf(file, "%*s" "},\n", indent, "");
        sv_save_items(file, indent);
    } indent -= 2; fprintf(file, "%*s" "}\n", indent, "");
    return true;
}
