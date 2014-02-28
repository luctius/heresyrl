#include "fov.h"

struct sgt_sight {
    fov_settings_type fov_settings;
};

static bool check_opaque(void *vmap, int x, int y) {
    struct 
}

static void apply_light_source(void *vmap, int x, int y, int dx, int dy, void *src) {
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

    int x = 0;
    int y = 0;
    if (item->owner_type == ITEM_OWNER_MAP) {
        x = item->owner.owner_map_entity.x_pos;
        y = item->owner.owner_map_entity.y_pos;
    }
    else if (item->owner_type == ITEM_OWNER_MONSTER) {
        x = item->owner.owner_monster.x_pos;
        y = item->owner.owner_monster.y_pos;
    }
    else return false;

    fov_circle(&sight->fov_settings, map, item, x, y, item->specific.tool.light_luminem);
    return true;
}

bool sgt_calculate_all_light_sources(struct sgt_sight *sight, struct dc_map *map) {

}

bool sgt_calculate_player_sight(struct sgt_sight *sight, struct dc_map *map, struct msr_monster *monster) {}

