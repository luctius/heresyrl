#include <math.h>

#include "sight.h"
#include "items.h"
#include "fov.h"
#include "tiles.h"
#include "dungeon_creator.h"
#include "monster.h"
#include "ai_utils.h"
#include "pathfinding.h"

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

static void apply_player_sight(void *vmap, int x, int y, int dx, int dy, void *isrc) {
    coord_t c = cd_create(x,y);
    struct dc_map *map = (struct dc_map *) vmap;
    struct msr_monster *monster = isrc;

    if (dc_verify_map(map) == false) return;
    if (cd_within_bound(&c, &map->size) == false) return;
    if (msr_verify_monster(monster) == false) return;

    struct dc_map_entity *me = sd_get_map_me(&c,map);
    int mod = 0;

    if (me->light_level > 0) {
        me->visible = true;
        me->discovered = true;
    }

    if (pyth(dx,dy) < msr_get_near_sight_range(monster)) {
        me->discovered = true;
        me->in_sight = true;
        me->visible = true;
    }
    else if (pyth(dx,dy) < msr_get_medium_sight_range(monster)) {
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
    fov_settings_set_apply_lighting_function(&sight->fov_settings, apply_player_sight);
    fov_circle(&sight->fov_settings, map, monster, monster->pos.x, monster->pos.y, msr_get_far_sight_range(monster) );
    apply_player_sight(map, monster->pos.x, monster->pos.y, 0, 0, monster);

    return true;
}

static int sgt_calc_path(coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz);
int sgt_los_path(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e, coord_t *path_list[]) {
    if (sight == NULL) return false;
    if (dc_verify_map(map) == false) return false;
    FIX_UNUSED(sight);

    int path_sz = cd_pyth(s,e);
    *path_list = calloc(path_sz +2, sizeof(coord_t) );
    if (*path_list == NULL) return -1;

    path_sz = sgt_calc_path(s, e, *path_list, path_sz);

    return path_sz;
}

struct check_los {
    coord_t *target;
    bool is_seen;
};

static void apply_has_los(void *vmap, int x, int y, int dx, int dy, void *isrc) {
    struct check_los *cl = isrc;
    struct dc_map *map = (struct dc_map *) vmap;

    coord_t c = cd_create(x,y);

    if ((cl->target->x == x) && (cl->target->y == y) ) {
        cl->is_seen = true;
    }
}

int dir_lot[3][3] = {
    {FOV_NORTHWEST, FOV_NORTH, FOV_NORTHEAST},
    {FOV_WEST,      -1,        FOV_EAST},
    {FOV_SOUTHWEST, FOV_SOUTH, FOV_SOUTHEAST}, };

bool sgt_has_los(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e) {
    if (sight == NULL) return false;
    if (dc_verify_map(map) == false) return false;

    struct check_los cl = { .target = e, .is_seen = false};

    fov_direction_type fdt;
    int fdt_x = 1;
    int fdt_y = 1;
    int dx = s->x - e->x;
    int dy = s->y - e->y;
    if (dx > 0) fdt_x = 0;
    else if (dx < 0) fdt_x = 2;
    if (dy > 0) fdt_y = 0;
    else if (dy < 0) fdt_y = 2;

    fdt = dir_lot[fdt_y][fdt_x];
    if (fdt == -1) return false;

    sight->fov_settings.corner_peek = FOV_CORNER_PEEK;
    fov_settings_set_opacity_test_function(&sight->fov_settings, check_opaque);
    fov_settings_set_apply_lighting_function(&sight->fov_settings, apply_has_los);
    fov_beam(&sight->fov_settings, map, &cl, s->x, s->y, cd_pyth(s,e) +1, fdt, 90);
    return cl.is_seen;
}

bool sgt_has_lof(struct sgt_sight *sight, struct dc_map *map, coord_t *s, coord_t *e) {
    return false;
}

/* Adapted from the code displayed at RogueBasin's "Bresenham's Line
 * Algorithm" article, this function checks for an unobstructed line
 * of sight between two locations using Bresenham's line algorithm to
 * draw a line from one point to the other. Returns true if there is
 * line of sight, false if there is no line of sight. */
static int sgt_calc_path(coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz) {
    int los_x_1 = s->x;
    int los_y_1 = s->y;
    int los_x_2 = e->x;
    int los_y_2 = e->y;
    int delta_x, delta_y, move_x, move_y, error;
    int pl_counter = 0;

    /* Calculate deltas. */
    delta_x = abs (los_x_2 - los_x_1) << 1;
    delta_y = abs (los_y_2 - los_y_1) << 1;

    /* Calculate signs. */
    move_x = los_x_2 >= los_x_1 ? 1 : -1;
    move_y = los_y_2 >= los_y_1 ? 1 : -1;

    /* There is an automatic line of sight, of course, between a
    * location and the same location or directly adjacent
    * locations. */
    if (abs (los_x_2 - los_x_1) < 2 && abs (los_y_2 - los_y_1) < 2) {
        /* Return. */
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "return %d", __LINE__);
        //return 0;
    }

    /* Ensure that the line will not extend too long. */
    if (((los_x_2 - los_x_1) * (los_x_2 - los_x_1))
    + ((los_y_2 - los_y_1) * (los_y_2 -
                             los_y_1)) > path_list_sz) {
        /* Return. */
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "fght", "return %d", __LINE__);
        //return -1;
    }

    /* "Draw" the line, checking for obstructions. */
    if (delta_x >= delta_y) {
        /* Calculate the error factor, which may go below zero. */
        error = delta_y - (delta_x >> 1);

        /* Search the line. */
        //while (los_x_1 != los_x_2) {
        while (pl_counter < path_list_sz) {
             /* Check for an obstruction. If the obstruction can be "moved
              * around", it isn't really an obstruction. */
            path_list[pl_counter].x = los_x_1;
            path_list[pl_counter].y = los_y_1;
            pl_counter++;
            //if (pl_counter == path_list_sz -1) return pl_counter;

            /* Update values. */
            if (error > 0) {
                if (error || (move_x > 0)) {
                    los_y_1 += move_y;
                    error -= delta_x;
                }
            }
            los_x_1 += move_x;
            error += delta_y;
        }
    }
    else {
        /* Calculate the error factor, which may go below zero. */
        error = delta_x - (delta_y >> 1);

        /* Search the line. */
        //while (los_y_1 != los_y_2) {
        while (pl_counter < path_list_sz) {
            /* Check for an obstruction. If the obstruction can be "moved
            * around", it isn't really an obstruction. */
            path_list[pl_counter].x = los_x_1;
            path_list[pl_counter].y = los_y_1;
            pl_counter++;
            if (pl_counter == path_list_sz -1) return pl_counter;

            /* Update values. */
            if (error > 0) {
                if (error || (move_y > 0)) {
                    los_x_1 += move_x;
                    error -= delta_y;
                }
            }
            los_y_1 += move_y;
            error += delta_x;
        }
    }

    path_list[pl_counter].x = los_x_2;
    path_list[pl_counter].y = los_y_2;
    pl_counter++;

    /* Return. */
    return pl_counter;
}
