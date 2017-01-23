/*
    This file is part of heresyRL.

    heresyRL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    heresyRL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with heresyRL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>

#include "heresyrl_def.h"

#include "tiles.h"
#include "random.h"
#include "ai/ai_utils.h"
#include "ai/pathfinding.h"
#include "inventory.h"
#include "items/items.h"
#include "items/items_static.h"
#include "fov/sight.h"
#include "options.h"
#include "turn_tick.h"

#include "dungeon/dungeon_map.h"
#include "dungeon/dungeon_helpers.h"
#include "dungeon/dungeon_cave.h"
#include "dungeon/dungeon_room.h"
#include "dungeon/dungeon_plain.h"
#include "dungeon/cellular_automata.h"
#include "dungeon/dungeon_dla.h"
#include "dungeon/dungeon_bsp.h"
#include "dungeon/dungeon_helpers.h"

#define LODEPNG_NO_COMPILE_DECODER
#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
#define LODEPNG_NO_COMPILE_CPP
#include "lodepng.h"
#include "heatmap.h"

extern inline struct dm_map_entity *dm_get_map_me(coord_t *c, struct dm_map *map);
extern inline struct tl_tile *dm_get_map_tile(coord_t *c, struct dm_map *map);
static bool dm_clear_map_unsafe(struct dm_map *map);

/*
   These checks are used to ensure that the structure passed is correct.
   Both in type and that another function has not overstepped its bounds.
 */
#define MAP_PRE_CHECK (13300)
#define MAP_POST_CHECK (3600)
#define MAPENTITY_PRE_CHECK (320)
#define MAPENTITY_POST_CHECK (7936)

static struct dm_map *dm_alloc_map(int x_sz, int y_sz) {
    if (x_sz < 2) return NULL;
    if (y_sz < 2) return NULL;

    int sz = sizeof(struct dm_map) + ( (x_sz) * y_sz * sizeof(*(((struct dm_map*)0)->map)) ) ;
    struct dm_map *map = calloc(sz, 1);
    if (map == NULL) return NULL;

    map->map_pre = MAP_PRE_CHECK;
    map->map_post = MAP_POST_CHECK;

    dm_clear_map_unsafe(map);

    return map;
}

bool dm_free_map(struct dm_map *map) {
    if (dm_verify_map(map) == false) return false;

    struct ground_effect *g = NULL;
    while ( (g = gelst_get_next(g) ) != NULL ) {
        ge_destroy(g->me);
        g = NULL;
    }

    coord_t c = cd_create(0,0);
    for (c.x = 0; c.x < map->sett.size.x; c.x++) {
        for (c.y = 0; c.y < map->sett.size.y; c.y++) {
            inv_exit(dm_get_map_me(&c,map)->inventory);
        }
    }

    free(map);
    return true;
}

/*
   This sets the map to default values.
 */
static bool dm_clear_map_unsafe(struct dm_map *map) {
    coord_t c = cd_create(0,0);
    for (c.x = 0; c.x < map->sett.size.x; c.x++) {
        for (c.y = 0; c.y < map->sett.size.y; c.y++) {
            dm_get_map_me(&c,map)->map_entity_pre = MAPENTITY_PRE_CHECK;
            dm_get_map_me(&c,map)->map_entity_post = MAPENTITY_POST_CHECK;

            dm_get_map_me(&c,map)->pos = c;
            dm_get_map_me(&c,map)->in_sight = false;
            dm_get_map_me(&c,map)->visible = false;
            dm_get_map_me(&c,map)->discovered = false;
            dm_get_map_me(&c,map)->light_level = 0;
            dm_get_map_me(&c,map)->monster = NULL;
            dm_get_map_me(&c,map)->icon_override = 0;
            dm_get_map_me(&c,map)->icon_attr_override = -1;
            dm_get_map_me(&c,map)->test_var = 0;

            if (dm_get_map_me(&c,map)->inventory != NULL) inv_exit(dm_get_map_me(&c,map)->inventory);
            dm_get_map_me(&c,map)->inventory = inv_init(inv_loc_tile);

            /*
                HACK: This is a hack to give map squares items based on their attributes.
                I should find a better place for this.
             */
            /*
            if (dm_get_map_tile(&c,map) != NULL) {
                if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&c,map), TILE_ATTR_LIGHT_SOURCE) ) {
                    struct itm_item *i = itm_create(IID_FIXED_LIGHT);
                    if (itm_insert_item(i, map, &c) == false) {
                        itm_destroy(i);
                        lg_debug("light failed at (%d,%d)", c.x,c.y);
                    }
                    else {
                        i->specific.tool.lit = true;
                        lg_debug("light at (%d,%d)", c.x,c.y);
                    }
                }
            }*/
        }
    }
    return true;
}

/*
   As with every verify function, check if the
   intergrity of the passed struct is correct.
 */
bool dm_verify_map(struct dm_map *map) {
    assert(map != NULL);
    assert(map->map_pre == MAP_PRE_CHECK);
    assert(map->map_post == MAP_POST_CHECK);
    assert(map->sett.size.x > 2);
    assert(map->sett.size.y > 2);
    assert(map->map != NULL);

    coord_t c = cd_create(0,0);
    struct dm_map_entity *me = dm_get_map_me(&c, map);
    assert(me->map_entity_pre = MAPENTITY_PRE_CHECK);
    assert(me->map_entity_post = MAPENTITY_POST_CHECK);

    c = cd_create(map->sett.size.x -1, map->sett.size.y -1);
    me = dm_get_map_me(&c, map);
    assert(me->map_entity_pre = MAPENTITY_PRE_CHECK);
    assert(me->map_entity_post = MAPENTITY_POST_CHECK);

    return true;
}

bool dm_print_map(struct dm_map *map) {
    if (dm_verify_map(map) == false) return false;

    printf("map seed: %" PRIu32 "\n", map->sett.seed);
    coord_t c;

    /* Heatmap test */
    heatmap_t* hm = heatmap_new(map->sett.size.x, map->sett.size.y);
    for (c.y = 0; c.y < map->sett.size.y; c.y++) {
        for (c.x = 0; c.x < map->sett.size.x; c.x++) {
            if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&c,map), TILE_ATTR_TRAVERSABLE) ) {
                heatmap_add_point(hm, c.x, c.y);
            }
        }
    }

    unsigned char image[map->sett.size.x * map->sett.size.y * 4];
    heatmap_render_default_to(hm, image);
    heatmap_free(hm);

    unsigned error = lodepng_encode32_file("test.png", image, map->sett.size.x, map->sett.size.y);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

    for (c.y = 0; c.y < map->sett.size.y; c.y++) {
        for (c.x = 0; c.x < map->sett.size.x; c.x++) {
            uint8_t r = image[ ( (((c.y) * map->sett.size.y) + (c.x) ) * 4) +0];
            uint8_t g = image[ ( (((c.y) * map->sett.size.y) + (c.x) ) * 4) +1];
            uint8_t b = image[ ( (((c.y) * map->sett.size.y) + (c.x) ) * 4) +2];
            uint8_t a = image[ ( (((c.y) * map->sett.size.y) + (c.x) ) * 4) +3];
            if ( (r >= 150) && 
                 (g >=   0  && 100 >= g) && 
                 (b >=   0  && 100 >= b) ) {
                printf("X");
            }
            else printf("%lc", dm_get_map_tile(&c,map)->icon);
        }
        if (c.y % 5 == 0) printf("\t -- %d\n", c.y);
        else putchar('\n');
    }
    putchar('\n');

    for (c.x = 0; c.x < map->sett.size.x; c.x++) {
        if (c.x % 10 == 0) putchar('|');
        else putchar(' ');
    }
    putwchar('\n');

/*
    for (c.x = 0; c.x < map->sett.size.x; c.x++) {
        for (c.y = 0; c.y < map->sett.size.y; c.y++) {
            uint8_t r = image[ ( (((c.x) * map->sett.size.y) + (c.y) ) * 4) +0];
            uint8_t g = image[ ( (((c.x) * map->sett.size.y) + (c.y) ) * 4) +1];
            uint8_t b = image[ ( (((c.x) * map->sett.size.y) + (c.y) ) * 4) +2];
            uint8_t a = image[ ( (((c.x) * map->sett.size.y) + (c.y) ) * 4) +3];
            if ( (r >= 150) && 
                 (g >=   0 &&  120 >= g) && 
                 (b >=   0 &&  70 >= g) ) {
                printf("XX");
            }
            else if (b >= 0xb0) printf("..");
            else printf("  ");
        }
        putchar('\n');
    }
    putchar('\n');
*/

    return true;
}

bool dm_tile_instance(struct dm_map *map, enum tile_types tt, int instance, coord_t *pos) {
    if (dm_verify_map(map) == false) return false;

    coord_t c;
    for (c.x = 0; c.x < map->sett.size.x; c.x++) {
        for (c.y = 0; c.y < map->sett.size.y; c.y++) {
            if (dm_get_map_tile(&c,map)->type == tt ) {
                instance--;
                if (instance <= 0) {
                    *pos = c;
                    return true;
                }
            }
        }
    }
    return false;
}

coord_t dm_scatter(struct dm_map *map, struct random *r) {
    int i_max = map->sett.size.x * map->sett.size.y / 4;
    coord_t p = cd_create(map->sett.size.x/2, map->sett.size.y/2);
    coord_t c = p;

    int i = 0;

    while (i < i_max) {
        i++;

        /* get a random point within radius */
        int dx = random_int32(r) % map->sett.size.x/2;
        int dy = random_int32(r) % map->sett.size.x/2;

        /* create the point relative to p */
        c = cd_create(p.x + dx, p.y +dy);

        /* require a point within map */
        if (cd_within_bound(&c, &map->sett.size) == false) continue;

        /* require an traversable point */
        if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&c,map), TILE_ATTR_TRAVERSABLE) == false) continue;

        /* require an tile without creatures */
        if (dm_get_map_me(&c,map)->monster != NULL) continue;

        /* we found a point which mathes our restrictions*/
        return c;
    }

    return cd_create(0,0);
}

bool dm_populate_map(struct dm_map *map) {
    if (dm_verify_map(map) == false) return false;
    coord_t c;
    int idx;

    /* init random*/
    struct random *r = random_init_genrand(map->sett.seed);
    if (r == NULL) return false;

    int nogo_radius = 30;

    for (int xi = 0; xi < map->sett.size.x; xi++) {
        for (int yi = 0; yi < map->sett.size.y; yi++) {
            c = cd_create(xi,yi);
            if (cd_pyth(&map->stair_up, &c) <= nogo_radius) continue; /* no npc's too close to the start */

            if ( (random_int32(r) % 10000) <= map->sett.monster_chance) {
                uint32_t mc = random_int32(r) % 100;
                if (mc <= 30) {
                    uint32_t leader = 0;
                    int msr_cnt = (random_int32(r) % 5) +1;
                    for (int i = 0; i < msr_cnt; i++) {
                        idx = msr_spawn(random_int32(r), map->sett.threat_lvl_min, map->sett.threat_lvl_max, map->sett.type);
                        assert(idx >= 0);
                        coord_t cp = sgt_scatter(map, r, &c, 10);

                        if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&cp,map), TILE_ATTR_TRAVERSABLE) == true) {
                            if (dm_get_map_me(&cp,map)->monster == NULL) {
                                struct msr_monster *monster = msr_create(idx);
                                msr_insert_monster(monster, map, &c);
                                msr_populate_inventory(monster, map->sett.threat_lvl_min, map->sett.threat_lvl_max, r);

                                ai_monster_init(monster, leader);
                                if (leader == 0) {
                                    leader = monster->uid;
                                    lg_debug("created swarm leader at (%d,%d)", cp.x, cp.y);
                                }
                                else lg_debug("created swarm member at (%d,%d)", cp.x, cp.y);
                            }
                        }
                    }
                }
                else if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&c,map), TILE_ATTR_TRAVERSABLE) == true) {
                    idx = msr_spawn(random_int32(r), map->sett.threat_lvl_min, map->sett.threat_lvl_max, map->sett.type);
                    assert(idx >= 0);
                    struct msr_monster *monster = msr_create(idx);
                    msr_insert_monster(monster, map, &c);
                    msr_populate_inventory(monster, map->sett.threat_lvl_min, map->sett.threat_lvl_max, r);

                    ai_monster_init(monster, 0);
                    lg_debug("spawning monster %s at (%d,%d)", monster->sd_name, c.x, c.y);
                }
            }
            else if ( (random_int32(r) % 10000) <= map->sett.item_chance) {
                if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&c,map), TILE_ATTR_TRAVERSABLE) == true) {
                    int range = map->sett.threat_lvl_max - map->sett.threat_lvl_min;
                    int item_level = (random_int32(r) % range) + map->sett.threat_lvl_min;
                    if ( (random_int32(r) % 100) > 95) item_level += 1;
                    if ( (random_int32(r) % 100) > 99) item_level += 1;
                    idx = itm_spawn(random_int32(r), item_level, ITEM_GROUP_ANY, NULL);
                    assert(idx >= 0);
                    struct itm_item *item = itm_create(idx);

                    itm_insert_item(item, map, &c);
                    lg_debug("spawning item %s at (%d,%d)", item->sd_name, c.x, c.y);
                }
            }
        }
    }

    free(r);
    return true;
}

/*
   TODO: seperate this into one stairs for up and one for down.
   Here we add stairs, supports only 2 at a time, for now.
   randomly selects a tile, puts down the up stairs,
   then tries a few times untill the distance is satisfying
   or untill we run out of tries.

   then it places the stairs.
 */
static void dm_add_stairs(struct dm_map *map, struct random *r) {
    if (dm_verify_map(map) == false) return;

    coord_t zero = cd_create(0,0);
    if (cd_equal(&map->stair_up,   &zero) == false) return;
    if (cd_equal(&map->stair_down, &zero) == false) return;

    struct tl_tile **tile_up = NULL;
    struct tl_tile **tile_down = NULL;
    struct tl_tile **tile_down_temp = NULL;
    int i = 0;
    int large_num = 10000;
    int target_distance = map->sett.size.x * 0.8f;

    int last_distance = 0;
    coord_t up = cd_create(0,0);
    coord_t down = cd_create(0,0);
    coord_t down_temp = cd_create(0,0);

    while (tile_up == NULL || tile_down == NULL) {
        coord_t c;
        c.x = random_int32(r) % map->sett.size.x;
        c.y = random_int32(r) % map->sett.size.y;
        i++;

        if (dm_get_map_tile(&c,map)->type == TILE_TYPE_FLOOR ) {
            if (tile_up == NULL) {
                tile_up = &dm_get_map_me(&c,map)->tile;
                up = c;
            }
            else if (tile_down == NULL) {
                int xdiff = (abs(c.x - up.x) );
                int ydiff = (abs(c.y - up.y) );
                int dist = 0;
                if ( (dist = pyth(xdiff, ydiff)) > target_distance) {
                    tile_down = &dm_get_map_me(&c,map)->tile;
                    down = c;
                }
                else if (dist > last_distance) {
                    tile_down_temp = &dm_get_map_me(&c,map)->tile;
                    last_distance = dist;
                    down_temp = c;
                }
            }
        }

        if ( (tile_down == NULL) && (i > large_num) ) {
            tile_down = tile_down_temp;
            i = 0;
            down = down_temp;
        }
    }
    *tile_up = ts_get_tile_type(TILE_TYPE_STAIRS_UP);
    *tile_down = ts_get_tile_type(TILE_TYPE_STAIRS_DOWN);

    map->stair_up = up;
    map->stair_down = down;
}

bool dm_clear_map(struct dm_map *map) {
    if (dm_verify_map(map) == false) return false;

    return dm_clear_map_unsafe(map);
}

/*
    resets visibility values.
 */
bool dm_clear_map_visibility(struct dm_map *map, coord_t *start, coord_t *end) {
    if (dm_verify_map(map) == false) return false;

    if (end->x > map->sett.size.x) return false;
    if (end->y > map->sett.size.y) return false;
    if (end->x+start->x > map->sett.size.x) return false;
    if (end->y+start->y > map->sett.size.y) return false;
    if (cd_within_bound(start, &map->sett.size) == false) return false;

    coord_t c = cd_add(start, end);
    for (c.x = start->x; c.x < end->x; c.x++) {
        for (c.y = start->y; c.y < end->y; c.y++) {
            struct dm_map_entity *me = dm_get_map_me(&c,map);
            me->in_sight = false;
            me->visible = false;
            me->light_level = 0;
            me->test_var = 0;
            me->icon_override = 0;
            me->icon_attr_override = -1;
        }
    }
    return true;
}

static bool dm_has_floors(struct dm_map *map) {
    coord_t c;

    for (c.x = 0; c.x < map->sett.size.x; c.x++) {
        for (c.y = 0; c.y < map->sett.size.y; c.y++) {
            if (TILE_HAS_ATTRIBUTE(dm_get_map_tile(&c, map),TILE_ATTR_TRAVERSABLE) ) return true;
        }
    }

    return false;
}

struct dm_map *dm_generate_map(struct dm_spawn_settings *sett) {
    struct dm_map *map = dm_alloc_map(sett->size.x, sett->size.y);
    memcpy(&map->sett, sett, sizeof(struct dm_spawn_settings) );
    if (dm_verify_map(map) == false) return NULL;

    lg_debug("generating map with seed \'%lu\', type \'%d\' and threat_lvl \'%d-%d\'",
            map->sett.seed, map->sett.type, map->sett.threat_lvl_min, map->sett.threat_lvl_max);

    coord_t c;
    /* Create an non-destructable border wall around the 4 sides of the map */
    for (c.x = 0; c.x < map->sett.size.x; c.x++) {
        for (c.y = 0; c.y < map->sett.size.y; c.y++) {
            if ( (c.x == 0) ||
                 (c.y == 0) ||
                 (c.x == map->sett.size.x-1) ||
                 (c.y == map->sett.size.y-1) ) {
                dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_BORDER_WALL);
            }
            else {
                dm_get_map_me(&c,map)->tile = ts_get_tile_specific(TILE_ID_CONCRETE_WALL);
            }
        }
    }

    /* save upper left and down right. */
    coord_t ul = { .x = 2, .y = 2,};
    coord_t dr = { .x = map->sett.size.x-3, .y = map->sett.size.y-3, };

    map->stair_up   = cd_create(0,0);
    map->stair_down = cd_create(0,0);

    /* init random*/
    struct random *r = random_init_genrand(map->sett.seed);

    if (map->sett.type == DUNGEON_TYPE_ALL) {
        map->sett.type = random_int32(r) % DUNGEON_TYPE_ALL;
    }

    struct dungeon_features_done *feat = NULL;

    /* fill the map with room accoring to the specified algorithm */
    switch(map->sett.type) {
        case DUNGEON_TYPE_CAVE:
            lg_print("map_type is cave");
            feat = dm_generate_map_cave(map, r, &ul, &dr);
            break;
        default:
        case DUNGEON_TYPE_PLAIN:
            lg_print("map_type is plain");
            feat = dm_generate_map_plain(map, r, &ul, &dr);
            break;
        case DUNGEON_TYPE_UNDERHIVE:
            lg_print("map_type is underhive");
            feat = dm_generate_map_dla(map, r, &ul, &dr);
            break;
        case DUNGEON_TYPE_HIVE:
            lg_print("map_type is hive");
            feat = dm_generate_map_bsp(map, r, &ul, &dr);
            break;
    }
    assert(feat != NULL);

    /* print map if requested */
    if (options.debug && options.print_map_only) {
        lg_debug("map before reachability");
        dm_print_map(map);
    }

    assert(dm_has_floors(map) );

    /* add the stairs to the map */
    dm_add_stairs(map, r);

    /* set map cells to their defaults. */
    dm_clear_map(map);

    /* Fill the map with lights */
    if (!feat->lights) dm_add_lights(map, r);

    /* From here we will make sure the map is completely accesible. */

    /* the start and end of the final check will be the up and down stairs. */
    coord_t start = { .x = map->stair_up.x, .y = map->stair_up.y};
    coord_t end = { .x = map->stair_down.x, .y = map->stair_down.y};

    bool map_is_good = feat->reachability ;
    struct pf_context *pf_ctx = NULL;

    int i = 0;
    for (i = 0; i < ( (map->sett.size.x * map->sett.size.y) / 100) && (map_is_good == false); i++) {
        /*
           We flood the map and rescue nonflooded segments untill we can find
           no more non-flooded tiles. This takes a long time though,
           and can probably be optimised.

           TODO: cache the generated dijkstra and only add new stuff. this
                 is /the/ performance killer for generating maps.
         */

        /* We generate a new flood map */
        if (aiu_generate_dijkstra(&pf_ctx, map, &start, 0) ) {

            /* check if there is no non-obstacle which has not been used in the map */
            if (pf_calculate_reachability(pf_ctx) == true) {
                map_is_good = true;

                /* Paranoia: check if there is a path from start to end */
                //assert(pf_calculate_path(pf_ctx, &start, &end, NULL) > 1);
            }
            else {
                /* if we do have regions which have not been recued, create a tunnel to them */
                dm_get_tunnel_path(map, pf_ctx, r);
            }

        }
    }
    assert(map_is_good == true);
    lg_debug("Map is completly reachable in %d tries", i);

    if (!feat->loops) dm_add_loops(map, pf_ctx, r);
/*
    if (!feat->features) {
        for (i = 0; i < 10; i++) {
            int x = random_int32(r) % map->sett.size.x;
            int y = random_int32(r) % map->sett.size.y;
            coord_t point = { .x = x, .y = y, };

            if (dm_generate_feature(map, r, &point, 15, 15, DM_FT_POOL) ) continue;
        }
    }
*/

    /* cleanup pathfinding */
    pf_exit(pf_ctx);

    /*cleanup random*/
    random_exit(r);

    if (options.print_map_only) {
        lg_debug("map after reachability");
        dm_print_map(map);
        dm_free_map(map);
        exit(EXIT_SUCCESS);
    }

    return map;
}

void dm_process_tiles(struct dm_map *map) {
    if (dm_verify_map(map) == false) return;

    /* Process tiles with additional effects. */
    coord_t c;
    for (c.x = 0; c.x < map->sett.size.x; c.x++) {
        for (c.y = 0; c.y < map->sett.size.y; c.y++) {
            struct dm_map_entity *me = dm_get_map_me(&c, map);
            if (me->monster != NULL) ts_turn_tick_monster(me->tile, me->monster);
            ts_turn_tick(me->tile, &c, map);
        }
    }
}

/* TODO: Think about ground based (status) effects and their livetime. */
bool dm_tile_enter(struct dm_map *map, coord_t *point, struct msr_monster *monster, coord_t *prev) {
    if (dm_verify_map(map) == false) return false;
    if (msr_verify_monster(monster) == false) return false;

    struct dm_map_entity *me = dm_get_map_me(point,map);
    assert(me->monster == NULL || me->monster == monster);

    struct tl_tile *prev_tile = dm_get_map_tile(prev, map);
    if (prev_tile != me->tile) {
        if (me->tile->plr_enter_str != NULL) You(monster,     "%ls", me->tile->plr_enter_str);
        //if (me->tile->msr_enter_str != NULL) Monster(monster, "%ls", me->tile->msr_enter_str);
    }

    if (me->effect != NULL) {
        se_add_status_effect(monster, me->effect->se_id, me->effect->sd_name);
    }

    ts_enter(me->tile, monster);

    return true;
}

bool dm_tile_exit(struct dm_map *map, coord_t *point, struct msr_monster *monster, coord_t *next) {
    if (dm_verify_map(map) == false) return false;
    if (msr_verify_monster(monster) == false) return false;

    struct dm_map_entity *me = dm_get_map_me(point,map);
    assert(me->monster == monster);
    me->monster = NULL;

    struct tl_tile *next_tile = dm_get_map_tile(next, map);
    if (next_tile != me->tile) {
        if (me->tile->plr_exit_str != NULL && next_tile->plr_enter_str == NULL) You(monster,     "%ls", me->tile->plr_exit_str);
        //if (me->tile->msr_exit_str != NULL) Monster(monster, "%ls", me->tile->msr_exit_str);
    }

    if (me->effect != NULL) {
        if ( (me->effect->flags & GR_EFFECTS_REMOVE_ON_EXIT) > 0) {
            se_remove_effects_by_tid(monster, me->effect->tid);
        }
    }

    ts_exit(me->tile, monster);

    return true;
}

bool dm_add_generator(struct generator *g) {
    return false;
}

void dm_init() {
}

void dm_exit() {
}
