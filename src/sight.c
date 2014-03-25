#include <math.h>

#include "sight.h"
#include "items.h"
#include "tiles.h"
#include "dungeon_creator.h"
#include "monster.h"
#include "ai_utils.h"
#include "pathfinding.h"
#include "game.h"
#include "fov.h"
#include "strict_fov.h"

/*
   We use libfov, recursive shadow casting for lightning
   and strict fov for player sight and los.
   strict is slow but symmetrical, and the los calculation 
   is identical then to the fov.
*/

struct sgt_sight {
    fov_settings_type fov_settings;
};

static bool check_opaque(struct dc_map *map, coord_t *point) {

    if (dc_verify_map(map) == false) return false;
    if (cd_within_bound(point, &map->size) == false) return false;

    return !( (sd_get_map_tile(point,map)->attributes & TILE_ATTR_OPAGUE) > 0);
}

static bool sf_check_opaque(struct strict_fov_set *set, coord_t *point, coord_t *origin) {
    struct dc_map *map = set->map;
    return check_opaque(map, point);
}

static bool sc_check_opaque(void *vmap, int x, int y) {
    struct dc_map *map = (struct dc_map *) vmap;

    coord_t c = cd_create(x,y);

    return check_opaque(map, &c);
}

static bool sf_apply_player_sight(struct strict_fov_set *set, coord_t *point, coord_t *origin) {
    struct dc_map *map = set->map;
    struct msr_monster *monster = set->source;

    if (dc_verify_map(map) == false) return false;
    if (cd_within_bound(point, &map->size) == false) return false;
    if (msr_verify_monster(monster) == false) return false;

    struct dc_map_entity *me = sd_get_map_me(point,map);
    int mod = 0;

    if (me->light_level > 0) {
        me->visible = true;
        me->discovered = true;
    }

    int range = cd_pyth(point, origin);
    if (range < msr_get_near_sight_range(monster)) {
        me->discovered = true;
        me->in_sight = true;
        me->visible = true;
    }
    else if (range < msr_get_medium_sight_range(monster)) {
        me->in_sight = true;
        me->discovered = true;
        mod = -20;
    }
    else {
        me->in_sight = true;
        mod = -30;
    }

    if (me->visible == false && me->monster != NULL) {
        if (msr_skill_check(monster, SKILLS_AWARENESS, mod) >= 0) {
            /*TODO scatter*/
            me->icon_override = '?';
        }
    }
    return true;
}
static void sc_apply_light_source(void *vmap, int x, int y, int dx, int dy, void *isrc) {
    struct dc_map *map = (struct dc_map *) vmap;
    struct itm_item *item = (struct itm_item *) isrc;

    coord_t c = cd_create(x,y);
    if (dc_verify_map(map) == false) return;
    if (cd_within_bound(&c, &map->size) == false) return;
    if (itm_verify_item(item) == false) return;
    if ( (item->specific.tool.light_luminem - pyth(dx, dy) ) <= 0) return;

    sd_get_map_me(&c,map)->light_level = item->specific.tool.light_luminem - pyth(dx, dy);
}

struct sgt_sight *sgt_init(void) {
    struct sgt_sight *retval = malloc(sizeof(struct sgt_sight) );
    if (retval != NULL) {
        fov_settings_init(&retval->fov_settings);
        retval->fov_settings.corner_peek = FOV_CORNER_PEEK;
        retval->fov_settings.opaque_apply = FOV_OPAQUE_APPLY;
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
    coord_t c = itm_get_pos(item);

    fov_settings_set_opacity_test_function(&sight->fov_settings, sc_check_opaque);
    fov_settings_set_apply_lighting_function(&sight->fov_settings, sc_apply_light_source);
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

    struct strict_fov_set set = {
        .source = monster,
        .map = map,
        .is_opaque = sf_check_opaque,
        .apply = sf_apply_player_sight,
    };

    strict_fov(&set, &monster->pos, msr_get_far_sight_range(monster) );
    return true;
}

struct los_apply_ctx {
    int dist_ctr;
    int max_dist;
    coord_t *plist;
};

static bool los_apply(struct strict_fov_set *set, coord_t *point, coord_t *origin) {
    struct los_apply_ctx *ctx = set->source;
    struct dc_map *map = set->map;

    if (ctx == NULL) return false;
    if (ctx->plist == NULL) return false;
    if (dc_verify_map(map) == false) return false;
    if (cd_within_bound(point, &map->size) == false) return false;
    if (ctx->dist_ctr >= ctx->max_dist) return false;
    ctx->plist[ctx->dist_ctr++] = *point;

    return true;
}

/*
int sgt_los_path(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e, coord_t *path_lst[], bool continue_path) {
    if (sight == NULL) return -1;
    if (dc_verify_map(map) == false) return -1;

    int d = cd_pyth(s,e);
    *path_lst = calloc(d, sizeof(coord_t) );

    return d;
}
*/

int sgt_los_path(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e, coord_t *path_lst[], bool continue_path) {
    if (sight == NULL) return -1;
    if (dc_verify_map(map) == false) return -1;
    coord_t end = *e;

    if (continue_path) {
        int dx = e->x - s->x;
        int dy = e->y - s->y;
        while ( (e->x > 0) && (e->y > 0) && (e->x < map->size.x) && (e->y < map->size.y) ) {
            e->x += dx;
            e->y += dy;
        }
    }

    struct los_apply_ctx actx = {
        .dist_ctr = 0,
        .max_dist = cd_pyth(s, &end) +2,
    };

    struct strict_fov_set set = {
        .source = &actx,
        .map = map,
        .is_opaque = sf_check_opaque,
        .apply = los_apply,
    };

    *path_lst = calloc(actx.max_dist +1, sizeof(coord_t) );
    if (*path_lst == NULL) return -1;
    actx.plist = *path_lst;

    if (strict_los(&set, s, &end, true) == false) {
        //free(*path_lst);
        //return -1;
    }
    return actx.dist_ctr;
}

bool sgt_has_los(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e) {
    if (sight == NULL) return false;
    if (dc_verify_map(map) == false) return false;

    struct strict_fov_set set = {
        .source = NULL,
        .map = map,
        .is_opaque = sf_check_opaque,
        .apply = NULL,
    };

    return strict_los(&set, s, e, false);
}

bool sgt_has_lof(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e) {
    if (sight == NULL) return false;
    if (dc_verify_map(map) == false) return false;

    struct strict_fov_set set = {
        .source = NULL,
        .map = map,
        .is_opaque = sf_check_opaque,
        .apply = NULL,
    };

    return strict_los(&set, s, e, false);
}

