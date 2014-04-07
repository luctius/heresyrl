#include <math.h>
#include <sys/param.h>
#include <assert.h>

#include "sight.h"
#include "fov.h"
#include "digital_fov.h"
#include "rpsc_fov.h"
#include "tiles.h"
#include "game.h"
#include "random.h"
#include "items/items.h"
#include "dungeon/dungeon_map.h"
#include "monster/monster.h"

struct sgt_sight {
    fov_settings_type fov_settings;
};

/* checks if this is a walkable path, without a monster.  */
static bool rpsc_check_opaque_lof(struct rpsc_fov_set *set, coord_t *point, coord_t *origin) {
    struct dm_map *map = set->map;

    FIX_UNUSED(origin);

    /* verify map structure */
    if (dm_verify_map(map) == false) return false;
    /* check if this point is within the map boundries. */
    if (cd_within_bound(point, &map->size) == false) return false;

    /* if the point is not traversable, return false */
    if ( (dm_get_map_tile(point,map)->attributes & TILE_ATTR_TRAVERSABLE) == 0) return false;

    /* if there is a monster, return false */
    if (dm_get_map_me(point,map)->monster != NULL) return false;

    /* it's a good lof point*/
    return true;
}

/* check if there is a line of sight path on this point */
static bool rpsc_check_opaque_los(struct rpsc_fov_set *set, coord_t *point, coord_t *origin) {
    struct dm_map *map = set->map;

    FIX_UNUSED(origin);

    /* verify map structure */
    if (dm_verify_map(map) == false) return false;
    /* check if this point is within the map boundries. */
    if (cd_within_bound(point, &map->size) == false) return false;

    /* if it is opague, return true, else return false. */
    return ( (dm_get_map_tile(point,map)->attributes & TILE_ATTR_OPAGUE) > 0);
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
        if ( (DoS = msr_skill_check(monster, SKILLS_AWARENESS, mod) ) >= 0) {
            /* the scatter radius decreases depending on the number of successes in our roll*/
            int radius = 4 - DoS;

            if (radius > 0) {
                /* if the roll was a success, scatter the blip. */
                coord_t sp = sgt_scatter(gbl_game->sight, map, gbl_game->game_random, point, radius);
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
    if ( (cd_equal(point, origin) == false) && ( (dm_get_map_tile(point,map)->attributes & TILE_ATTR_OPAGUE) == 0) ) return false;

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
    if ( (dm_get_map_tile(point,map)->attributes & TILE_ATTR_TRAVERSABLE) == 0) return false;

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
    //if ( (dm_get_map_tile(point,map)->attributes & TILE_ATTR_TRAVERSABLE) == 0) return false;

    pp->list[pp->list_idx++] = *point;
    lg_debug("added point (%d,%d) to idx %d", point->x, point->y, pp->list_idx-1);

    return true;
}

struct sgt_sight *sgt_init(void) {
    struct sgt_sight *retval = malloc(sizeof(struct sgt_sight) );
    if (retval != NULL) {
        /*
        fov_settings_init(&retval->fov_settings);
        retval->fov_settings.corner_peek = FOV_CORNER_PEEK;
        retval->fov_settings.opaque_apply = FOV_OPAQUE_APPLY;
        */
    }
    return retval;
}

void sgt_exit(struct sgt_sight *sight) {
    if (sight != NULL) {
        //fov_settings_free(&sight->fov_settings);
        free(sight);
    }
}

bool sgt_calculate_light_source(struct sgt_sight *sight, struct dm_map *map, struct itm_item *item) {
    if (sight == NULL) return false;
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
        .is_opaque = rpsc_check_opaque_los,
        .apply = rpsc_apply_light_source,
    };

    rpsc_fov(&set, &c, item->specific.tool.light_luminem);
    return true;
}

bool sgt_calculate_all_light_sources(struct sgt_sight *sight, struct dm_map *map) {
    if (sight == NULL) return false;
    if (dm_verify_map(map) == false) return false;

    struct itm_item *item = NULL;
    while ( (item = itmlst_get_next_item(item) ) != NULL){
        sgt_calculate_light_source(sight, map, item);
    }
    return true;
}

bool sgt_calculate_player_sight(struct sgt_sight *sight, struct dm_map *map, struct msr_monster *monster) {
    if (sight == NULL) return false;
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
        .is_opaque = rpsc_check_opaque_los,
        .apply = rpsc_apply_player_sight,
    };

    rpsc_fov(&set, &monster->pos, msr_get_far_sight_range(monster) );
    return true;
}

int sgt_explosion(struct sgt_sight *sight, struct dm_map *map, coord_t *pos, int radius, coord_t *grid_list[]) {
    if (sight == NULL) return false;
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
        .is_opaque = rpsc_check_opaque_los,
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

int sgt_los_path(struct sgt_sight *sight, struct dm_map *map, coord_t *s, coord_t *e, coord_t *path_lst[], bool continue_path, int radius) {
    if (sight == NULL) return false;
    if (dm_verify_map(map) == false) return false;

    if (cd_equal(s,e) == true) return 0;

    /* allocate the total number of grids within the path.
     this should be way to many but at this point we do not 
     now how many there are.
     */
    *path_lst = calloc(cd_pyth(s,e) * 2, sizeof(coord_t) );
    if (*path_lst == NULL) return -1;


    /*create the projectile path struct, containing 
      - a list for the grids for the path
      - the maximum size of that list.
      - a index counter for that list.
     */
    struct sgt_projectile_path_struct ex = {
        .list = *path_lst,
        .list_sz = cd_pyth(s,e) * 2,
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
        .is_opaque = rpsc_check_opaque_lof,
        .apply = rpsc_apply_projectile_path,
    };

    if (rpsc_los(&set, s, e, radius) == false || ex.list_idx == 0) {
        free (*path_lst);
        return -1;
    }

    /* 
       we are probably not using a lot of the space allocated.
       thus we request a smaller block. If we do that correctly,
       we should save the old pointer, but we do not because we
       are lazy. assert for now that it is succesfull.
     */
    *path_lst = realloc(*path_lst, ex.list_idx * sizeof(coord_t) );
    assert(*path_lst != NULL);
    return ex.list_idx;
}
coord_t sgt_scatter(struct sgt_sight *sight, struct dm_map *map, struct random *r, coord_t *p, int radius) {
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
            if ( (dm_get_map_tile(&c,map)->attributes & TILE_ATTR_TRAVERSABLE) == 0) continue;

            /* require line of sight */
            if (sgt_has_los(sight, map, p, &c, radius) == false) continue;
            
            /* we found a point which mathes our restrictions*/
            break;
        }
    }

    /* return the (perhaps modified) point. */
    return c;
}

bool sgt_has_los(struct sgt_sight *sight, struct dm_map *map, coord_t *s, coord_t *e, int radius) {
    if (sight == NULL) return false;
    if (dm_verify_map(map) == false) return false;

    struct rpsc_fov_set set = {
        .source = s,
        .permissiveness = RPSC_FOV_PERMISSIVE_NORMAL,
        .area = RPSC_AREA_CIRCLE,
        .map = map,
        .size = map->size,
        .is_opaque = rpsc_check_opaque_los,
        .apply = NULL,
    };

    return rpsc_los(&set, s, e, radius);
}

bool sgt_has_lof(struct sgt_sight *sight, struct dm_map *map, coord_t *s, coord_t *e, int radius) {
    if (sight == NULL) return false;
    if (dm_verify_map(map) == false) return false;

    struct rpsc_fov_set set = {
        .source = s,
        .permissiveness = RPSC_FOV_PERMISSIVE_NORMAL,
        .area = RPSC_AREA_CIRCLE,
        .map = map,
        .size = map->size,
        .is_opaque = rpsc_check_opaque_lof,
        .apply = NULL,
    };

    return rpsc_los(&set, s, e, radius);
}

#if 0
int bresenham(struct dm_map *map, coord_t *s, coord_t *e, coord_t plist[], int plist_sz) {
    int x_1 = s->x;
    int y_1 = s->y;
    int x_2 = e->x;
    int y_2 = e->y;
    int plist_idx = 0;
    int delta_x = (x_2 - x_1);
    // if x_1 == x_2, then it does not matter what we set here
    signed char const ix = ((delta_x > 0) - (delta_x < 0));
    delta_x = abs(delta_x) << 1;

    int delta_y = (y_2 - y_1);
    // if y_1 == y_2, then it does not matter what we set here
    signed char const iy = ((delta_y > 0) - (delta_y < 0));
    delta_y = abs(delta_y) << 1;

    plist[plist_idx++] = *s;
    if (plist_idx >= plist_sz) return -1;

    if (delta_x >= delta_y)
    {
        // error may go below zero
        int error = (delta_y - (delta_x >> 1));

        while (x_1 != x_2)
        {
            if ((error >= 0) && (error || (ix > 0)))
            {
                error -= delta_x;
                y_1 += iy;
            }
            // else do nothing

            error += delta_y;
            x_1 += ix;

            plist[plist_idx++] = cd_create(x_1,y_1);
            if ( (dm_get_map_tile(&plist[plist_idx-1],map)->attributes & TILE_ATTR_TRAVERSABLE) == 0) return -1;
            if (plist_idx >= plist_sz) return -1;
        }
    }
    else
    {
        // error may go below zero
        int error = (delta_x - (delta_y >> 1));

        while (y_1 != y_2)
        {
            if ((error >= 0) && (error || (iy > 0)))
            {
                error -= delta_y;
                x_1 += ix;
            }
            // else do nothing

            error += delta_x;
            y_1 += iy;

            plist[plist_idx++] = cd_create(x_1,y_1);
            if ( (dm_get_map_tile(&plist[plist_idx-1],map)->attributes & TILE_ATTR_TRAVERSABLE) == 0) return -1;
            if (plist_idx >= plist_sz) return -1;
        }
    }

    return plist_idx;
}

#define ipart_(X) ((int)(X))
#define round_(X) ((int)(((double)(X))+0.5))
#define fpart_(X) (((double)(X))-(double)ipart_(X))
#define rfpart_(X) (1.0-fpart_(X))
 
#define swap_(a, b) do{ __typeof__(a) tmp;  tmp = a; a = b; b = tmp; }while(0)
int wu_line(coord_t *s, coord_t *e, coord_t plst[], int plst_sz) {
  unsigned int x_1 = s->x;
  unsigned int y_1 = s->y;
  unsigned int x_2 = e->x;
  unsigned int y_2 = e->y;
  double dx = (double)x_2 - (double)x_1;
  double dy = (double)y_2 - (double)y_1;

  int length = 0;

  if ( fabs(dx) > fabs(dy) ) {
    if ( x_2 < x_1 ) {
      swap_(x_1, x_2);
      swap_(y_1, y_2);
    }

    if (dx == 0) return -1;

    double gradient = dy / dx;
    double xend = round_(x_1);
    double yend = y_1 + gradient*(xend - x_1);
    int xpxl1 = xend;
    int ypxl1 = ipart_(yend);

    plst[length++] = cd_create(xpxl1,ypxl1);
    plst[length++] = cd_create(xpxl1,ypxl1+1);

    double intery = yend + gradient;
 
    xend = round_(x_2);
    yend = y_2 + gradient*(xend - x_2);
    int xpxl2 = xend;
    int ypxl2 = ipart_(yend);

    plst[length++] = cd_create(xpxl2,ypxl2);
    plst[length++] = cd_create(xpxl2,ypxl2+1);
 
    int x;
    for(x=xpxl1+1; x <= (xpxl2-1); x++) {
      plst[length++] = cd_create(x,ipart_(intery) );
      plst[length++] = cd_create(x,ipart_(intery) +1);

      intery += gradient;

    }
  } else {
    if ( y_2 < y_1 ) {
      swap_(x_1, x_2);
      swap_(y_1, y_2);
    }

    if (dy == 0) return -1;

    double gradient = dx / dy;
    double yend = round_(y_1);
    double xend = x_1 + gradient*(yend - y_1);
    int ypxl1 = yend;
    int xpxl1 = ipart_(xend);

    plst[length++] = cd_create(xpxl1,ypxl1);
    plst[length++] = cd_create(xpxl1,ypxl1+1);

    double interx = xend + gradient;
 
    yend = round_(y_2);
    xend = x_2 + gradient*(yend - y_2);
    int ypxl2 = yend;
    int xpxl2 = ipart_(xend);

    plst[length++] = cd_create(xpxl2,ypxl2);
    plst[length++] = cd_create(xpxl2,ypxl2+1);
 
    int y;
    for(y=ypxl1+1; y <= (ypxl2-1); y++) {
      plst[length++] = cd_create(ipart_(interx), y);
      plst[length++] = cd_create(ipart_(interx) +1, y);

      interx += gradient;
    }
  }

  return length;
}
#undef swap_
#undef plot_
#undef ipart_
#undef fpart_
#undef round_
#undef rfpart_
#endif
