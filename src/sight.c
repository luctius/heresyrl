#include <math.h>
#include <sys/param.h>

#include "sight.h"
#include "items.h"
#include "tiles.h"
#include "dungeon_creator.h"
#include "monster.h"
#include "ai_utils.h"
#include "pathfinding.h"
#include "game.h"
#include "fov.h"
#include "digital_fov.h"


struct sgt_sight {
    fov_settings_type fov_settings;
};

static bool check_opaque(struct dc_map *map, coord_t *point) {

    if (dc_verify_map(map) == false) return false;
    if (cd_within_bound(point, &map->size) == false) return false;

    return !( (sd_get_map_tile(point,map)->attributes & TILE_ATTR_OPAGUE) > 0);
}

static bool dig_check_opaque_lof(struct digital_fov_set *set, coord_t *point, coord_t *origin) {
    struct dc_map *map = set->map;

    if (dc_verify_map(map) == false) return false;
    if (cd_within_bound(point, &map->size) == false) return false;

    if ( (sd_get_map_tile(point,map)->attributes & TILE_ATTR_OPAGUE) == 0) return false;
    if (sd_get_map_me(point,map)->monster != NULL) return false;
    return true;
}

static bool dig_check_opaque_los(struct digital_fov_set *set, coord_t *point, coord_t *origin) {
    struct dc_map *map = set->map;

    if (dc_verify_map(map) == false) return false;
    if (cd_within_bound(point, &map->size) == false) return false;

    return ( (sd_get_map_tile(point,map)->attributes & TILE_ATTR_OPAGUE) > 0);
}

static bool sc_check_opaque(void *vmap, int x, int y) {
    struct dc_map *map = (struct dc_map *) vmap;

    coord_t c = cd_create(x,y);

    return check_opaque(map, &c);
}

static bool dig_apply_player_sight(struct digital_fov_set *set, coord_t *point, coord_t *origin) {
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

static bool dig_apply_light_source(struct digital_fov_set *set, coord_t *point, coord_t *origin) {
    struct dc_map *map = set->map;
    struct itm_item *item = set->source;

    if (dc_verify_map(map) == false) return false;
    if (cd_within_bound(point, &map->size) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if ( (item->specific.tool.light_luminem - cd_pyth(point, origin) ) <= 0) return false;

    sd_get_map_me(point,map)->light_level = item->specific.tool.light_luminem - cd_pyth(point, origin);
    return true;
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

    /*
    fov_settings_set_opacity_test_function(&sight->fov_settings, sc_check_opaque);
    fov_settings_set_apply_lighting_function(&sight->fov_settings, sc_apply_light_source);
    fov_circle(&sight->fov_settings, map, item, c.x, c.y, item->specific.tool.light_luminem);
    */

    struct digital_fov_set set = {
        .source = item,
        .map = map,
        .size = map->size,
        .is_opaque = dig_check_opaque_los,
        .apply = dig_apply_light_source,
    };

    digital_fov(&set, &c, item->specific.tool.light_luminem);
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

    struct digital_fov_set set = {
        .source = monster,
        .map = map,
        .size = map->size,
        .is_opaque = dig_check_opaque_los,
        .apply = dig_apply_player_sight,
    };

    digital_fov(&set, &monster->pos, msr_get_far_sight_range(monster) );
    return true;
}

struct los_apply_ctx {
    int dist_ctr;
    int max_dist;
    coord_t *plist;
};

static bool dig_los_apply(struct digital_fov_set *set, coord_t *point, coord_t *origin) {
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

static bool find_projectile_path_rec_backtr(coord_t blocklst[], int bl_idx, coord_t plist[], int pl_idx, coord_t *s, coord_t *e) {
    /*
       We have a list of coords from which we want to make a smooth line.
       we search the next n coords or untill we hit one which is too far.
       we try to get a dx +dy as big as possible, but not one which brings 
       us farther from s than we are.
     */

    if (pl_idx == 1) {
        lg_debug("done");
        return true;
    }

    coord_t *point = &plist[pl_idx];

    lg_debug("I'm (%d,%d) %d, %d", point->x, point->y, bl_idx, pl_idx);

    while (true) {
        int best_idx = -1;
        float best_dist = 1000;
        for (int i = 0; i <= MIN(8, bl_idx); i++) {
            coord_t *cp = &blocklst[bl_idx - i];
            coord_t c_dist = cd_delta_abs(cp, point);

            lg_debug("testing[%d] (%d,%d)", bl_idx - i, cp->x, cp->y);
            if (c_dist.x <= 1 && c_dist.y <= 1 && cd_equal(point, cp) == false) {
                coord_t bda = cd_delta_abs(cp, s);
                coord_t pda = cd_delta_abs(point, s);

                lg_debug("checking if i'm not making it worse (%d,%d)", cp->x, cp->y);
                if ( (bda.x <= pda.x) && (bda.y <= pda.y) ) {
                    lg_debug("Nope, it's good");
                    float d1 = sqrt( ( (cp->x - s->x) * (cp->x - s->x) ) * ( (cp->y - s->y) * (cp->y - s->y) ) );
                    if (d1 < best_dist) {
                        lg_debug("this is the best! (%d,%d)", cp->x, cp->y);
                        plist[pl_idx-1] = *cp;
                        best_dist = d1;
                        best_idx = bl_idx - i;
                    }
                    else lg_debug("%f >= %f :S", d1, best_dist);
                }
            }
        }

        if (best_idx == -1) return false;

        coord_t bda = cd_delta_abs(&blocklst[best_idx], s);
        coord_t pda = cd_delta_abs(point, s);
        if ( (bda.x > pda.x) || (bda.y > pda.y) ) {
            lg_debug("this 'best' is _really_ bad..");
            return false;
        }

        if (find_projectile_path_rec_backtr(blocklst, best_idx, plist, pl_idx-1,s,e) == false) {
            blocklst[best_idx].x = -100;
            blocklst[best_idx].y = -100;
            best_dist = 1000;
            lg_debug("best was a dead end");
        }
        else return true;
    }

    return false;
}

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
        .max_dist = cd_pyth(s, &end) * 4,
    };

    struct digital_fov_set set = {
        .source = &actx,
        .map = map,
        .size = map->size,
        .is_opaque = dig_check_opaque_los,
        .apply = dig_los_apply,
    };

    coord_t *pathblock_lst = calloc(actx.max_dist *2, sizeof(coord_t) );
    if (pathblock_lst == NULL) return -1;
    actx.plist = pathblock_lst;

    if (cd_pyth(s,e) > 1) {
        if (digital_los(&set, s, &end, true) == false) {
            free(pathblock_lst);
            return -1;
        }
    }
    else actx.dist_ctr = 1;

    if (actx.dist_ctr > 0) {
        int dist = cd_pyth(s,&end) +1;
        *path_lst = calloc(dist, sizeof(coord_t) );
        if (*path_lst == NULL) {
            free(pathblock_lst);
            return -1;
        }

        if (actx.dist_ctr > 1) {
            (*path_lst)[dist-1] = end;
            find_projectile_path_rec_backtr(pathblock_lst, actx.dist_ctr-2, *path_lst, dist-1,s, &end);
        }
        (*path_lst)[0] = *s;
    }
    free(pathblock_lst);


    return cd_pyth(s, &end)+1;
}

bool sgt_has_los(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e) {
    if (sight == NULL) return false;
    if (dc_verify_map(map) == false) return false;

    struct digital_fov_set set = {
        .source = NULL,
        .map = map,
        .size = map->size,
        .is_opaque = dig_check_opaque_los,
        .apply = NULL,
    };

    return digital_los(&set, s, e, false);
}

bool sgt_has_lof(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e) {
    if (sight == NULL) return false;
    if (dc_verify_map(map) == false) return false;

    struct digital_fov_set set = {
        .source = NULL,
        .map = map,
        .size = map->size,
        .is_opaque = dig_check_opaque_lof,
        .apply = NULL,
    };

    return digital_los(&set, s, e, false);
}

