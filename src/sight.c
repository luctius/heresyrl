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

//#define SHADOW_FOV
#define DIGITAL_FOV

struct sgt_sight {
    fov_settings_type fov_settings;
};

#ifdef DIGITAL_FOV
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
        lg_print("Awareness check on (%d,%d)", point->x, point->y);
        if (msr_skill_check(monster, SKILLS_AWARENESS, mod) >= 0) {
            /*TODO scatter*/
            me->icon_override = '?';
        }
    }
    return true;
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
#endif

#ifdef SHADOW_FOV
static bool sc_check_opaque(void *vmap, int x, int y) {
    struct dc_map *map = (struct dc_map *) vmap;

    coord_t c = cd_create(x,y);
    if (dc_verify_map(map) == false) return false;
    if (cd_within_bound(&c, &map->size) == false) return false;

    return !( (sd_get_map_tile(&c,map)->attributes & TILE_ATTR_OPAGUE) > 0);
}

static void sc_apply_player_sight(void *vmap, int x, int y, int dx, int dy, void *vsrc) {
    struct dc_map *map = vmap;
    struct msr_monster *monster = vsrc;
    coord_t point = cd_create(x,y);

    if (dc_verify_map(map) == false) return;
    if (cd_within_bound(&point, &map->size) == false) return;
    if (msr_verify_monster(monster) == false) return;

    struct dc_map_entity *me = sd_get_map_me(&point,map);
    int mod = 0;

    if (me->light_level > 0) {
        me->visible = true;
        me->discovered = true;
    }

    int range = pyth(dx, dy);
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
        lg_print("Awareness check on (%d,%d)", point.x, point.y);
        if (msr_skill_check(monster, SKILLS_AWARENESS, mod) >= 0) {
            /*TODO scatter*/
            me->icon_override = '?';
        }
    }
    return;
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
#endif

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

#ifdef SHADOW_FOV
    /*
    fov_settings_set_opacity_test_function(&sight->fov_settings, sc_check_opaque);
    fov_settings_set_apply_lighting_function(&sight->fov_settings, sc_apply_light_source);
    fov_circle(&sight->fov_settings, map, item, c.x, c.y, item->specific.tool.light_luminem);
    */
#endif

#ifdef DIGITAL_FOV
    struct digital_fov_set set = {
        .source = item,
        .map = map,
        .size = map->size,
        .is_opaque = dig_check_opaque_los,
        .apply = dig_apply_light_source,
    };

    digital_fov(&set, &c, item->specific.tool.light_luminem);
#endif

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

#ifdef SHADOW_FOV
    fov_settings_set_opacity_test_function(&sight->fov_settings, sc_check_opaque);
    fov_settings_set_apply_lighting_function(&sight->fov_settings, sc_apply_player_sight);
    fov_circle(&sight->fov_settings, map, monster, monster->pos.x, monster->pos.y, msr_get_far_sight_range(monster) );
    sc_apply_player_sight(map, monster->pos.x, monster->pos.y, 0,0, monster);
#endif

#ifdef DIGITAL_FOV
    struct digital_fov_set set = {
        .source = monster,
        .map = map,
        .size = map->size,
        .is_opaque = dig_check_opaque_los,
        .apply = dig_apply_player_sight,
    };

    digital_fov(&set, &monster->pos, msr_get_far_sight_range(monster) );
#endif

    return true;
}

#ifdef DIGITAL_FOV
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

    if (sgt_has_los(sight, map, s,e) == false) return -1;

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
#endif

#ifdef SHADOW_FOV
int bresenham(struct dc_map *map, int x1, int y1, int const x2, int const y2, coord_t plist[], int plist_sz);

int sgt_los_path(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e, coord_t *path_lst[], bool continue_path) {
    if (sight == NULL) return -1;
    if (dc_verify_map(map) == false) return -1;
    coord_t begin = *s;
    coord_t end = *e;

    if (sgt_has_los(sight, map, s,e) == false) return -1;

    if (continue_path) {
        int dx = e->x - s->x;
        int dy = e->y - s->y;
        while ( (e->x > 0) && (e->y > 0) && (e->x < map->size.x) && (e->y < map->size.y) ) {
            begin.x += dx;
            begin.y += dy;
        }
    }

    coord_t delta[] = { {0,0},
        {-1,1},  {0,1},  {1,1},
        {-1,0},          {1,0},
        {-1,-1}, {0,-1}, {1,-1},
    };

    int path_max_sz = (cd_pyth(s, e) +8) * 2;
    coord_t initial_path_list[path_max_sz];

    /*
       We brute force ourselves into a projectile path from a start point
       and an endpoint *around* the given points (we test those first btw).

       When we have a valid path, travel it backwards to find the optimal one.
     */
    bool found = false;
    int path_sz = -1;
    int init_path_sz = -1;
    for (int i = 0; i < ARRAY_SZ(delta) && (found == false); i++) {
        lg_debug("try %d from (%d,%d) => (%d,%d)", i, s->x,s->y,e->x,e->y);
        end.x = e->x + delta[i].x;
        end.y = e->y + delta[i].y;
        if (cd_within_bound(&begin, &map->size) == false) continue;
        if (cd_within_bound(&end, &map->size) == false) continue;

        init_path_sz = bresenham(map, end.x, end.y,s->x, s->y, initial_path_list, path_max_sz);
        if (init_path_sz < 0) {
            lg_debug("begin (%d,%d) end (%d,%d) not good", s->x, s->y, end.x,end.y);
        }
        else {
            found = true;
            lg_debug("begin (%d,%d) end (%d,%d) *is* good", s->x, s->y, end.x,end.y);
        }
    }
    
    if (found == false) {
        lg_debug("hopeless");
        return -1;
    }

    for (int i = init_path_sz -1; i >= 0; i--) {
        lg_debug("initplist[%d] (%d,%d)", i, initial_path_list[i].x, initial_path_list[i].y);
    }

    path_sz = cd_pyth(s,e) +2;
    *path_lst = calloc(path_sz +1, sizeof(coord_t) );
    if (*path_lst == NULL) return -1;
    coord_t *plist = *path_lst;

    plist[path_sz -1] = *e;
    for (int i = path_sz -2; i >= 0; i--) {
        int ipl_idx = 0;
        coord_t last = plist[i+1];
        coord_t *best = &last;
        coord_t src_dist = cd_delta_abs(best, s);

        /* consider the next 3 points in the array */
        for (int j = ipl_idx; j < initial_path_list && j <= ipl_idx+3; j++) {
            coord_t *cp = &initial_path_list[j];
            coord_t c_dist = cd_delta_abs(cp, &last);

            /* the step cannot be greater than dx: +1, dy: +1*/
            if (c_dist.x <= 1 && c_dist.y <= 1) {
                c_dist = cd_delta_abs(cp, s);
                if (cd_pyth(cp,s) < cd_pyth(best,s) ) {
                    best = cp;
                    src_dist = c_dist;
                }
                else if (cd_pyth(cp,s) == cd_pyth(best,s) ) {
                    if ( (c_dist.x < src_dist.x) || (c_dist.y < src_dist.y) ) {
                        best = cp;
                        src_dist = c_dist;
                        ipl_idx = j+1;
                    }
                }
            }
        }
        plist[i] = *best;

        if (src_dist.x <= 1 && src_dist.y <= 1) {
            i = -1;
        }
    }
    plist[0] = *s;

    return path_sz;
}

#endif

bool sgt_has_los(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e) {
    if (sight == NULL) return false;
    if (dc_verify_map(map) == false) return false;

#ifdef SHADOW_FOV
    return false;

#elif defined (DIGITAL_FOV)
    struct digital_fov_set set = {
        .source = NULL,
        .map = map,
        .size = map->size,
        .is_opaque = dig_check_opaque_los,
        .apply = NULL,
    };

    return digital_los(&set, s, e, false);
#endif
}

bool sgt_has_lof(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e) {
    if (sight == NULL) return false;
    if (dc_verify_map(map) == false) return false;

#ifdef SHADOW_FOV
    return false;

#elif defined(DIGITAL_FOV)
    struct digital_fov_set set = {
        .source = NULL,
        .map = map,
        .size = map->size,
        .is_opaque = dig_check_opaque_lof,
        .apply = NULL,
    };

    return digital_los(&set, s, e, false);
#endif
}


int bresenham(struct dc_map *map, int x1, int y1, int const x2, int const y2, coord_t plist[], int plist_sz) {
    int plist_idx = 0;
    int delta_x = (x2 - x1);
    // if x1 == x2, then it does not matter what we set here
    signed char const ix = ((delta_x > 0) - (delta_x < 0));
    delta_x = abs(delta_x) << 1;

    int delta_y = (y2 - y1);
    // if y1 == y2, then it does not matter what we set here
    signed char const iy = ((delta_y > 0) - (delta_y < 0));
    delta_y = abs(delta_y) << 1;

    if (plist_idx < plist_sz) plist[plist_idx++] = cd_create(x1,y1);

    if (delta_x >= delta_y)
    {
        // error may go below zero
        int error = (delta_y - (delta_x >> 1));

        while (x1 != x2)
        {
            if ((error >= 0) && (error || (ix > 0)))
            {
                error -= delta_x;
                y1 += iy;
            }
            // else do nothing

            error += delta_y;
            x1 += ix;

            coord_t point = cd_create(x1,y1);
            if ( (sd_get_map_tile(&point,map)->attributes & TILE_ATTR_OPAGUE) > 0) {
                if (plist_idx < plist_sz) {
                    plist[plist_idx++] = point;
                    sd_get_map_me(&point,map)->test_var = 2;
                }
            }
            else return -1;
        }
    }
    else
    {
        // error may go below zero
        int error = (delta_x - (delta_y >> 1));

        while (y1 != y2)
        {
            if ((error >= 0) && (error || (iy > 0)))
            {
                error -= delta_y;
                x1 += ix;
            }
            // else do nothing

            error += delta_x;
            y1 += iy;

            coord_t point = cd_create(x1,y1);
            if ( (sd_get_map_tile(&point,map)->attributes & TILE_ATTR_OPAGUE) > 0) {
                if (plist_idx < plist_sz) {
                    plist[plist_idx++] = point;
                    sd_get_map_me(&point,map)->test_var = 2;
                }
            }
            else return -1;
        }
    }
    return plist_idx;
}

