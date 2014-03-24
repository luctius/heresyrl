#include <math.h>

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

    coord_t c = cd_create(x,y);
    if (dc_verify_map(map) == false) return false;
    if (cd_within_bound(&c, &map->size) == false) return false;

    return !( (sd_get_map_tile(&c,map)->attributes & TILE_ATTR_OPAGUE) > 0);
}

static void apply_light_source(void *vmap, int x, int y, int dx, int dy, void *isrc) {
    struct dc_map *map = (struct dc_map *) vmap;
    struct itm_item *item = (struct itm_item *) isrc;

    coord_t c = cd_create(x,y);
    if (dc_verify_map(map) == false) return;
    if (cd_within_bound(&c, &map->size) == false) return;
    if (itm_verify_item(item) == false) return;
    if ( (item->specific.tool.light_luminem - pyth(dx, dy) ) <= 0) return;

    sd_get_map_me(&c,map)->light_level = item->specific.tool.light_luminem - pyth(dx, dy);
}

static void apply_far_player_sight(void *vmap, int x, int y, int dx, int dy, void *isrc) {
    struct dc_map *map = (struct dc_map *) vmap;
    struct msr_monster *monster = (struct msr_monster *) isrc;
    int radius = msr_get_medium_sight_range(monster);
    if (pyth(dx,dy) < radius) return; /*we are now applying far sight; ignore everything in medum and direct sight*/

    coord_t c = cd_create(x,y);

    sd_get_map_me(&c,map)->in_sight = true;
    if (sd_get_map_me(&c,map)->light_level > 0) {
        sd_get_map_me(&c,map)->discovered = true;
        sd_get_map_me(&c,map)->visible = true;
    }
    else {
        if (sd_get_map_me(&c,map)->monster != NULL) {
            if (msr_skill_check(monster, SKILLS_AWARENESS, -30) >= 0) {
                /*TODO scatter*/
                sd_get_map_me(&c,map)->icon_override = '?';
            }
        }
    }
}

static void apply_medium_player_sight(void *vmap, int x, int y, int dx, int dy, void *isrc) {
    struct dc_map *map = (struct dc_map *) vmap;
    struct msr_monster *monster = (struct msr_monster *) isrc;
    int radius = msr_get_near_sight_range(monster);
    if (pyth(dx,dy) < radius) return; /*we are now applying medium sight; ignore everything in direct sight*/

    coord_t c = cd_create(x,y);

    sd_get_map_me(&c,map)->in_sight = true;
    sd_get_map_me(&c,map)->discovered = true;
    if (sd_get_map_me(&c,map)->light_level > 0) {
        sd_get_map_me(&c,map)->visible = true;
    }
    else {
        if (sd_get_map_me(&c,map)->monster != NULL) {
            if (msr_skill_check(monster, SKILLS_AWARENESS, -20) >= 0) {
                /*TODO scatter*/
                sd_get_map_me(&c,map)->icon_override = '?';
            }
        }
    }
}

static void apply_near_player_sight(void *vmap, int x, int y, int dx, int dy, void *isrc) {
    struct dc_map *map = (struct dc_map *) vmap;

    coord_t c = cd_create(x,y);
    sd_get_map_me(&c,map)->discovered = true;
    sd_get_map_me(&c,map)->in_sight = true;
    sd_get_map_me(&c,map)->visible = true;
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

bool sgt_calculate_light_source(struct sgt_sight *sight, struct dc_map *map, struct itm_item *item) {
    if (sight == NULL) return false;
    if (dc_verify_map(map) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (tool_is_type(item, TOOL_TYPE_LIGHT) == false) return false;
    if (item->specific.tool.lit != true) return false;

    //lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "sight", "processing light source %s.", item->ld_name);
    fov_settings_set_opacity_test_function(&sight->fov_settings, check_opaque);
    fov_settings_set_apply_lighting_function(&sight->fov_settings, apply_light_source);

    coord_t c = itm_get_pos(item);

    fov_circle(&sight->fov_settings, map, item, c.x, c.y, item->specific.tool.light_luminem);
    sd_get_map_me(&c,map)->icon_attr_override = get_colour(TERM_COLOUR_YELLOW);
    return true;
}

bool sgt_calculate_all_light_sources(struct sgt_sight *sight, struct dc_map *map) {
    if (sight == NULL) return false;
    if (dc_verify_map(map) == false) return false;

    struct itm_item *item = NULL;
    while ( (item = itmlst_get_next_item(item) ) != NULL){
        sgt_calculate_light_source(sight, map, item);
    }
    return true;
}

bool sgt_calculate_player_sight(struct sgt_sight *sight, struct dc_map *map, struct msr_monster *monster) {
    if (sight == NULL) return false;
    if (dc_verify_map(map) == false) return false;
    if (msr_verify_monster(monster) == false) return false;

    fov_settings_set_opacity_test_function(&sight->fov_settings, check_opaque);
    fov_settings_set_apply_lighting_function(&sight->fov_settings, apply_near_player_sight);
    fov_circle(&sight->fov_settings, map, monster, monster->pos.x, monster->pos.y, msr_get_near_sight_range(monster) );
    apply_near_player_sight(map, monster->pos.x, monster->pos.y, 0, 0, monster);

    fov_settings_set_opacity_test_function(&sight->fov_settings, check_opaque);
    fov_settings_set_apply_lighting_function(&sight->fov_settings, apply_medium_player_sight);
    fov_circle(&sight->fov_settings, map, monster, monster->pos.x, monster->pos.y, msr_get_far_sight_range(monster) );

    fov_settings_set_opacity_test_function(&sight->fov_settings, check_opaque);
    fov_settings_set_apply_lighting_function(&sight->fov_settings, apply_far_player_sight);
    fov_circle(&sight->fov_settings, map, monster, monster->pos.x, monster->pos.y, msr_get_far_sight_range(monster) );
    return true;
}

