#include "sight.h"
#include "items.h"
#include "fov.h"
#include "tiles.h"
#include "dungeon_creator.h"
#include "monster.h"

struct sgt_sight {
    fov_settings_type fov_settings;
};

static bool check_opaque(void *vmap, int x, int y) {
    struct dc_map *map = (struct dc_map *) vmap;
    if (map == NULL) return false;

    coord_t c = cd_create(x,y);
    return !( (sd_get_map_tile(&c,map)->attributes & TILE_ATTR_OPAGUE) > 0);
}

static void apply_light_source(void *vmap, int x, int y, int dx, int dy, void *isrc) {
    struct dc_map *map = (struct dc_map *) vmap;
    struct itm_items *item = (struct itm_items *) isrc;
    if (map == NULL) return;
    if (item == NULL) return;

    coord_t c = cd_create(x,y);
    sd_get_map_me(&c,map)->light_level = item->specific.tool.light_luminem - pyth(dx,dy);
}

static void apply_player_sight(void *vmap, int x, int y, int dx, int dy, void *isrc) {
    struct dc_map *map = (struct dc_map *) vmap;
    struct msr_monster *monster = (struct msr_monster *) isrc;

    coord_t c = cd_create(x,y);
    /* Do some checks against monster on location to see if it is visible, later...*/
    sd_get_map_me(&c,map)->discovered = true;
    sd_get_map_me(&c,map)->in_sight = true;

    int radius = msr_get_near_sight_range(monster) +msr_get_far_sight_range(monster);
    if (pyth(dx, dy) < radius) {
        sd_get_map_me(&c,map)->visible = true;
    }
    else if (sd_get_map_me(&c,map)->light_level > 0) {
        sd_get_map_me(&c,map)->visible = true;
    }
}

struct sgt_sight *sgt_init(void) {
    struct sgt_sight *retval = malloc(sizeof(struct sgt_sight) );
    if (retval != NULL) {
        fov_settings_init(&retval->fov_settings);
    }
    return retval;
}

void sgt_exit(struct sgt_sight *sight) {
    if (sight != NULL) {
        fov_settings_free(&sight->fov_settings);
        free(sight);
    }
}

bool sgt_calculate_light_source(struct sgt_sight *sight, struct dc_map *map, struct itm_items *item) {
    if (sight == NULL) return false;
    if (map == NULL) return false;
    if (item == NULL) return false;
    if (item->item_type != ITEM_TYPE_TOOL) return false;
    if (item->specific.tool.tool_type != ITEM_TOOL_TYPE_LIGHT) return false;
    if (item->specific.tool.lit != true) return false;

    fov_settings_set_opacity_test_function(&sight->fov_settings, check_opaque);
    fov_settings_set_apply_lighting_function(&sight->fov_settings, apply_light_source);

    coord_t c = itm_get_pos(item);

    fov_circle(&sight->fov_settings, map, item, c.x, c.y, item->specific.tool.light_luminem);
    return true;
}

bool sgt_calculate_all_light_sources(struct sgt_sight *sight, struct dc_map *map) {
    if (sight == NULL) return false;
    if (map == NULL) return false;

    struct itm_items *item = NULL;
    while ( (item = itmlst_get_next_item(item) ) != NULL){
        sgt_calculate_light_source(sight, map, item);
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "sight", "processing light source %s.", item->ld_name);
    }
    return true;
}

bool sgt_calculate_player_sight(struct sgt_sight *sight, struct dc_map *map, struct msr_monster *monster) {
    if (sight == NULL) return false;
    if (map == NULL) return false;
    if (monster == NULL) return false;

    fov_settings_set_opacity_test_function(&sight->fov_settings, check_opaque);
    fov_settings_set_apply_lighting_function(&sight->fov_settings, apply_player_sight);

    int radius = msr_get_near_sight_range(monster) +msr_get_far_sight_range(monster);
    fov_circle(&sight->fov_settings, map, monster, monster->pos.x, monster->pos.y, radius);
    check_opaque(map, monster->pos.x, monster->pos.y);
    apply_player_sight(map, monster->pos.x, monster->pos.y, 0, 0, monster);
    return true;
}

