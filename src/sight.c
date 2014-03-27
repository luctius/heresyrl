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

#define SHADOW_FOV
//#define DIGITAL_FOV

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

int bresenham(struct dc_map *map, coord_t *s, coord_t *e, coord_t plist[], int plist_sz);
int wu_line(coord_t *s, coord_t *e, coord_t plst[], int plst_sz);

int sgt_los_path(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e, coord_t *path_lst[], bool continue_path) {
    if (sight == NULL) return -1;
    if (dc_verify_map(map) == false) return -1;
    coord_t begin = *s;
    coord_t end = *e;

    if (sd_get_map_me(s,map)->visible == false) return -1;
    if (sd_get_map_me(e,map)->visible == false) return -1;
    if ( (sd_get_map_tile(s,map)->attributes & TILE_ATTR_OPAGUE) == 0) return -1;
    if ( (sd_get_map_tile(e,map)->attributes & TILE_ATTR_OPAGUE) == 0) return -1;

    int path_sz = (cd_pyth(s,e) +2 )* 4;
    coord_t wlst[path_sz];
    coord_t blst1[path_sz];
    coord_t blst2[path_sz];
    int blst1_sz = 0;
    int blst2_sz = 0;

    int wlst_sz = wu_line(s,e, wlst, path_sz);
    if (wlst_sz == -1) {
        lg_debug("wu line failed");
        wlst_sz = 1;
        wlst[0] = *s;
    }

    bool found = false;
    for (int i = 0; (i < wlst_sz) && (found == false); i++) {
       blst1_sz = bresenham(map, s, &wlst[i], blst1, path_sz);
       blst2_sz = bresenham(map, &wlst[i], e, blst2, path_sz);
       if ( (blst1_sz > 0) && (blst2_sz > 0) ) found = true;
    }

    if (found == false) return -1;

    path_sz = blst1_sz + blst2_sz +2;
    *path_lst = calloc(path_sz, sizeof(coord_t) );
    if (*path_lst == NULL) return -1;

    int pidx = 0;
    for (int i = 0; i < blst1_sz; i++, pidx++) {
        (*path_lst)[pidx] = blst1[i];
    }

    for (int i = 1; i < blst2_sz; i++, pidx++) {
        (*path_lst)[pidx] = blst2[i];
    }

    lg_debug("good");
    return pidx;
}

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


int bresenham(struct dc_map *map, coord_t *s, coord_t *e, coord_t plist[], int plist_sz) {
    int x1 = s->x;
    int y1 = s->y;
    int x2 = e->x;
    int y2 = e->y;
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

            plist[plist_idx++] = cd_create(x1,y1);
            if ( (sd_get_map_tile(&plist[plist_idx-1],map)->attributes & TILE_ATTR_OPAGUE) == 0) return -1;
            if (plist_idx >= plist_sz) return -1;
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

            plist[plist_idx++] = cd_create(x1,y1);
            if ( (sd_get_map_tile(&plist[plist_idx-1],map)->attributes & TILE_ATTR_OPAGUE) == 0) return -1;
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
  unsigned int x1 = s->x;
  unsigned int y1 = s->y;
  unsigned int x2 = e->x;
  unsigned int y2 = e->y;
  double dx = (double)x2 - (double)x1;
  double dy = (double)y2 - (double)y1;

  int length = 0;
  if (dx == dy) return -1;

  if ( fabs(dx) > fabs(dy) ) {
    if ( x2 < x1 ) {
      swap_(x1, x2);
      swap_(y1, y2);
    }
    double gradient = dy / dx;
    double xend = round_(x1);
    double yend = y1 + gradient*(xend - x1);
    double xgap = rfpart_(x1 + 0.5);
    int xpxl1 = xend;
    int ypxl1 = ipart_(yend);

    plst[length++] = cd_create(xpxl1,ypxl1);
    plst[length++] = cd_create(xpxl1,ypxl1+1);

    double intery = yend + gradient;
 
    xend = round_(x2);
    yend = y2 + gradient*(xend - x2);
    xgap = fpart_(x2+0.5);
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
    if ( y2 < y1 ) {
      swap_(x1, x2);
      swap_(y1, y2);
    }
    double gradient = dx / dy;
    double yend = round_(y1);
    double xend = x1 + gradient*(yend - y1);
    double ygap = rfpart_(y1 + 0.5);
    int ypxl1 = yend;
    int xpxl1 = ipart_(xend);

    plst[length++] = cd_create(xpxl1,ypxl1);
    plst[length++] = cd_create(xpxl1,ypxl1+1);

    double interx = xend + gradient;
 
    yend = round_(y2);
    xend = x2 + gradient*(yend - y2);
    ygap = fpart_(y2+0.5);
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
