#include <math.h>
#include <sys/param.h>
#include <assert.h>

#include "sight.h"
#include "rpsc_fov.h"
#include "tiles.h"
#include "game.h"
#include "random.h"
#include "items/items.h"
#include "dungeon/dungeon_map.h"
#include "monster/monster.h"
#include "status_effects/status_effects.h"

/* checks if this is a walkable path, without a monster.  */
static bool rpsc_check_translucent_lof(struct rpsc_fov_set *set, coord_t *point, coord_t *origin) {
    struct dm_map *map = set->map;

    FIX_UNUSED(origin);

    /* verify map structure */
    if (dm_verify_map(map) == false) return false;
    /* check if this point is within the map boundries. */
    if (cd_within_bound(point, &map->size) == false) return false;

    /* if the point is not traversable, return false */
    if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(point,map), TILE_ATTR_TRAVERSABLE) == false) return false;

    /* if there is a monster, return false */
    if (dm_get_map_me(point,map)->monster != NULL) return false;

    /* it's a good lof point*/
    return true;
}

/* check if there is a line of sight path on this point */
static bool rpsc_check_translucent_los(struct rpsc_fov_set *set, coord_t *point, coord_t *origin) {
    struct dm_map *map = set->map;
    bool translucent = true;

    FIX_UNUSED(origin);

    /* verify map structure */
    if (dm_verify_map(map) == false) return false;
    /* check if this point is within the map boundries. */
    if (cd_within_bound(point, &map->size) == false) return false;

    /* if it is translucent, return true, else return false. */
    if ( (dm_get_map_tile(point,map)->attributes & TILE_ATTR_TRANSLUCENT) == 0) {
        translucent = false;
    }
    else if (dm_get_map_me(point, map)->status_effect != NULL) {
        struct status_effect *se = dm_get_map_me(point, map)->status_effect;
        if (se_verify_status_effect(se) ) {
            translucent = se_has_flag(se, SEF_BLOCKS_SIGHT);
        }
    }

    return translucent;
}

static bool rpsc_apply_player_sight(struct rpsc_fov_set *set, coord_t *point, coord_t *origin) {
    struct dm_map *map = set->map;
    struct msr_monster *monster = set->source;

    /* verify map structure */
    if (dm_verify_map(map) == false) return false;
    /* check if this point is within map boundries. */
    if (cd_within_bound(point, &map->size) == false) return false;
    /*verify monster structure */
    if (msr_verify_monster(monster) == false) return false;

    /* get map entity*/
    struct dm_map_entity *me = dm_get_map_me(point,map);
    /* awareness check difficulty starts at zero */
    int mod = 0;
    /*every map point touched here is in sight.*/
    me->in_sight = true;

    if (me->light_level > 0) {
        /* if there is light, we can see everything.*/
        me->visible = true;
        me->discovered = true;
    }

    int range = cd_pyth(point, origin);
    if (range < msr_get_near_sight_range(monster)) {
        /* if it is in our near sight, we can see everything.*/
        me->discovered = true;
        me->visible = true;
    }
    else if (range < msr_get_medium_sight_range(monster)) {
        /* in our medium sight we can see the map features.*/
        me->discovered = true;
        mod = -20;
    }
    else {
        /* in our far sight without light, we have a chance of seeing movemnt (monsters).*/
        mod = -30;
    }

    /* check if we can see a monster in the dark */
    if (me->visible == false && me->monster != NULL) {
        lg_print("Awareness check on (%d,%d)", point->x, point->y);
        int DoS = 0;
        /*do an awareness check*/
        if ( (DoS = msr_skill_check(monster, MSR_SKILLS_AWARENESS, mod) ) >= 0) {
            /* the scatter radius decreases depending on the number of successes in our roll*/
            int radius = 4 - DoS;

            if (radius > 0) {
                /* if the roll was a success, scatter the blip. */
                coord_t sp = sgt_scatter(map, gbl_game->random, point, radius);
                dm_get_map_me(&sp, map)->icon_override = '?';
            }
        }
    }
    return true;
}

static bool rpsc_apply_light_source(struct rpsc_fov_set *set, coord_t *point, coord_t *origin) {
    struct dm_map *map = set->map;
    struct itm_item *item = set->source;

    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(point, &map->size) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if ( (item->specific.tool.light_luminem - cd_pyth(point, origin) ) <= 0) return false;

    /* Only light walls who are the origin of the light. */
    if ( (cd_equal(point, origin) == false) && ( (dm_get_map_tile(point,map)->attributes & TILE_ATTR_TRANSLUCENT) == 0) ) return false;

    dm_get_map_me(point,map)->light_level = item->specific.tool.light_luminem - cd_pyth(point, origin);
    return true;
}

struct sgt_explosion_struct {
    coord_t *list;
    short list_sz;
    short list_idx;
};

static bool rpsc_apply_explosion(struct rpsc_fov_set *set, coord_t *point, coord_t *origin) {
    struct dm_map *map = set->map;
    struct sgt_explosion_struct *ex = set->source;

    FIX_UNUSED(origin);

    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(point, &map->size) == false) return false;
    if (ex->list_idx >= ex->list_sz) return false;
    if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(point,map), TILE_ATTR_TRAVERSABLE) == false) return false;

    ex->list[ex->list_idx++] = *point;

    return true;
}

struct sgt_projectile_path_struct {
    coord_t *list;
    short list_sz;
    short list_idx;
};

static bool rpsc_apply_projectile_path(struct rpsc_fov_set *set, coord_t *point, coord_t *origin) {
    struct dm_map *map = set->map;
    struct sgt_projectile_path_struct *pp = set->source;

    FIX_UNUSED(origin);

    if (dm_verify_map(map) == false) return false;
    if (cd_within_bound(point, &map->size) == false) return false;
    if (pp->list_idx >= pp->list_sz) return false;
    if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(point,map), TILE_ATTR_TRAVERSABLE) == false) return false;

    pp->list[pp->list_idx++] = *point;
    lg_debug("added point (%d,%d) to idx %d", point->x, point->y, pp->list_idx-1);

    return true;
}

bool sgt_calculate_light_source(struct dm_map *map, struct itm_item *item) {
    if (dm_verify_map(map) == false) return false;
    if (itm_verify_item(item) == false) return false;
    if (tool_is_type(item, TOOL_TYPE_LIGHT) == false) return false;
    if (item->specific.tool.lit != true) return false;
    coord_t c = itm_get_pos(item);

    struct rpsc_fov_set set = {
        .source = item,
        .permissiveness = RPSC_FOV_PERMISSIVE_NORMAL,
        .area = RPSC_AREA_CIRCLE,
        .visible_on_equal = true,
        .not_visible_blocks_vision = true,
        .map = map,
        .size = map->size,
        .is_translucent = rpsc_check_translucent_los,
        .apply = rpsc_apply_light_source,
    };

    rpsc_fov(&set, &c, item->specific.tool.light_luminem);
    return true;
}

bool sgt_calculate_all_light_sources(struct dm_map *map) {
    if (dm_verify_map(map) == false) return false;

    struct itm_item *item = NULL;
    while ( (item = itmlst_get_next_item(item) ) != NULL){
        sgt_calculate_light_source(map, item);
    }
    return true;
}

bool sgt_calculate_player_sight(struct dm_map *map, struct msr_monster *monster) {
    if (dm_verify_map(map) == false) return false;
    if (msr_verify_monster(monster) == false) return false;

    struct rpsc_fov_set set = {
        .source = monster,
        .permissiveness = RPSC_FOV_PERMISSIVE_NORMAL,
        .area = RPSC_AREA_CIRCLE,
        .visible_on_equal = true,
        .not_visible_blocks_vision = true,
        .map = map,
        .size = map->size,
        .is_translucent = rpsc_check_translucent_los,
        .apply = rpsc_apply_player_sight,
    };

    rpsc_fov(&set, &monster->pos, msr_get_far_sight_range(monster) );
    return true;
}

int sgt_explosion(struct dm_map *map, coord_t *pos, int radius, coord_t *grid_list[]) {
    if (dm_verify_map(map) == false) return false;

    /*handle case of radius zero*/
    if (radius == 0) {
        *grid_list = calloc(1, sizeof(coord_t) );
        (*grid_list)[0] = *pos;
        return 1;
    }

    /* allocate the total number of grids within the explosion radius.
     this should be way to many but at this point we do not now how 
     many there are.
     */
    int sz = (radius * 4) * (radius * 4);
    *grid_list = calloc(sz, sizeof(coord_t) );
    if (*grid_list == NULL) return -1;


    /*create the explosion struct, containing 
      - a list for the grids within the explosion
      - the maximum size of that list.
      - a index counter for that list.
     */
    struct sgt_explosion_struct ex = {
        .list = *grid_list,
        .list_sz = sz,
        .list_idx = 0,
    };

    struct rpsc_fov_set set = {
        .source = &ex,
        .permissiveness = RPSC_FOV_PERMISSIVE_NORMAL,
        .area = RPSC_AREA_CIRCLE,
        .visible_on_equal = true,
        .not_visible_blocks_vision = true,
        .map = map,
        .size = map->size,
        .is_translucent = rpsc_check_translucent_los,
        .apply = rpsc_apply_explosion,
    };

    rpsc_fov(&set, pos, radius);

    /* 
       we are probably not using a lot of the space allocated.
       thus we request a smaller block. If we do that correctly,
       we should save the old pointer, but we do not because we
       are lazy. assert for now that it is succesfull.
     */
    *grid_list = realloc(*grid_list, ex.list_idx * sizeof(coord_t) );
    assert(*grid_list != NULL);
    return ex.list_idx;
}

int sgt_los_path(struct dm_map *map, coord_t *s, coord_t *e, coord_t *path_lst[], bool continue_path) {
    if (dm_verify_map(map) == false) return -1;

    /* if start and end are equal, or if end is a wall, bailout. */
    if (cd_equal(s,e) == true) return 0;
    if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(e,map), TILE_ATTR_TRAVERSABLE) == false) return -1;

    /* allocate the total number of grids within the path.
     this should be way to many but at this point we do not 
     now how many there are.
     */
    int psz = cd_pyth(s,e) * 2;
    if (continue_path) {
        psz = MAX(map->size.x, map->size.y);
    }

    *path_lst = calloc(psz, sizeof(coord_t) );
    if (*path_lst == NULL) return -1;


    /*create the projectile path struct, containing 
      - a list for the grids for the path
      - the maximum size of that list.
      - a index counter for that list.
     */
    struct sgt_projectile_path_struct pp = {
        .list = *path_lst,
        .list_sz = psz,
        .list_idx = 0,
    };

    struct rpsc_fov_set set = {
        .source = &pp,
        .permissiveness = RPSC_FOV_PERMISSIVE_NORMAL,
        .area = RPSC_AREA_CIRCLE,
        .visible_on_equal = true,
        .not_visible_blocks_vision = true,
        .map = map,
        .size = map->size,
        .is_translucent = rpsc_check_translucent_lof,
        .apply = rpsc_apply_projectile_path,
    };

    if (rpsc_los(&set, s, e) == false || pp.list_idx == 0) {
        free (*path_lst);
        return -1;
    }

    if (continue_path && psz > 1) {
        /*
            get a point from the original list,
            take the delta of it and its predecessor,
            add that to the previous point we put in
            at the end of the list. This creates a
            continues path untill an obstacle is found.
        */
        int i = pp.list_idx;
        bool blocked = false;
        int j = i;
        while ( (i < psz) && (blocked == false) ) {
            if ( (i % pp.list_idx) == 0) { i++; continue; }

            /* take a point from the original line */
            coord_t point = (*path_lst)[i % pp.list_idx];
            /* take the point before that */
            coord_t point_prev = (*path_lst)[ (i-1) % pp.list_idx];
            /* take the last point calculated. */
            coord_t point_last = (*path_lst)[j-1];

            /*calc the difference between the 2 points of the original line*/
            coord_t d = cd_delta(&point, &point_prev);

            /* apply that difference to our last point,
            creating a new point with the same offset.*/
            point.x = point_last.x + d.x;
            point.y = point_last.y + d.y;

            /* put that point into our new list*/
            (*path_lst)[j] = point;

            i++;
            j++;

            /*continue untill we are blocked.*/
            if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&point,map), TILE_ATTR_TRAVERSABLE) == false) {
                blocked = true;
                psz = j;
            }
        }
    }

    /* 
       we are probably not using a lot of the space allocated.
       thus we request a smaller block. If we do that correctly,
       we should save the old pointer, but we do not because we
       are lazy. assert for now that it is succesfull.
     */
    *path_lst = realloc(*path_lst, psz * sizeof(coord_t) );
    assert(*path_lst != NULL);
    return psz;
}

coord_t sgt_scatter(struct dm_map *map, struct random *r, coord_t *p, int radius) {
    /* Do not try forever. */
    int i_max = radius * radius;
    coord_t c = *p;

    if (radius > 0) {
        int i = 0;

        while (i < i_max) {
            i++;

            /* get a random point within radius */
            int dx = random_int32(r) % radius;
            int dy = random_int32(r) % radius;

            /* create the point relative to p */
            c = cd_create(p->x + dx, p->y +dy);

            /* require a point within map */
            if (cd_within_bound(&c, &map->size) == false) continue;

            /* require an traversable point */
            if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&c,map), TILE_ATTR_TRAVERSABLE) == false) continue;

            /* require line of sight */
            if (sgt_has_los(map, p, &c, radius) == false) continue;
            
            /* we found a point which mathes our restrictions*/
            return c;
        }
    }

    /* return the (perhaps modified) point. */
    return *p;
}

bool sgt_has_los(struct dm_map *map, coord_t *s, coord_t *e, int radius) {
    if (dm_verify_map(map) == false) return false;

    struct rpsc_fov_set set = {
        .source = s,
        .permissiveness = RPSC_FOV_PERMISSIVE_NORMAL,
        .area = RPSC_AREA_CIRCLE,
        .map = map,
        .size = map->size,
        .is_translucent = rpsc_check_translucent_los,
        .apply = NULL,
    };

    if (rpsc_in_radius(&set, s, e, radius) == false) return false;
    return rpsc_los(&set, s, e);
}

bool sgt_has_lof(struct dm_map *map, coord_t *s, coord_t *e, int radius) {
    if (dm_verify_map(map) == false) return false;

    struct rpsc_fov_set set = {
        .source = s,
        .permissiveness = RPSC_FOV_PERMISSIVE_NORMAL,
        .area = RPSC_AREA_CIRCLE,
        .map = map,
        .size = map->size,
        .is_translucent = rpsc_check_translucent_lof,
        .apply = NULL,
    };

    if (rpsc_in_radius(&set, s, e, radius) == false) return false;
    return rpsc_los(&set, s, e);
}

