#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pathfinding.h"
#include "heresyrl_def.h"

enum pf_entity_state {
    PF_ENTITY_STATE_FREE    = 0,
    PF_ENTITY_STATE_OPEN    = 1,
    PF_ENTITY_STATE_CLOSED  = 2,
};

struct pf_map_entity {
    int distance;
    unsigned int cost;
    unsigned int score;
    enum pf_entity_state state;
};

struct pf_map {
    struct pf_coord size;
    struct pf_map_entity *map;
};

struct pf_context {
    struct pf_map map;
    struct pf_settings set;
    int maximum_distance;
};

static inline struct pf_map_entity *pf_get_index(struct pf_coord *p, struct pf_map *map) {
    return &map->map[(p->x * map->size.y) + p->y];
}

static const struct pf_coord pf_coord_lo_table[] = {
    {-1,-1}, {-1,0}, {-1,1}, 
    { 0,-1},         { 0,1}, 
    { 1,-1}, { 1,0}, { 1,1},
};

static bool pf_flood_map_point(struct pf_context *ctx, struct pf_coord *point, struct pf_coord *target) {
    if (ctx == NULL) return false;
    if (point == NULL) return false;
    if (target == NULL) return false;
    if ( (point->x < 0) || (point->x >= ctx->map.size.x) ) return false;
    if ( (point->y < 0) || (point->y >= ctx->map.size.y) ) return false;
    if ( (point->x == target->x) && (point->y == target->y) ) return true;

    struct pf_map *map = &ctx->map;
    struct pf_map_entity *me = pf_get_index(point, map);
    if (me->state == PF_ENTITY_STATE_CLOSED) return false;
    if (me->distance >= ctx->maximum_distance) return false;
    me->state = PF_ENTITY_STATE_CLOSED;
    //lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf", "entering (%d,%d) -> [st: %d/ cst: %d/dst: %d]", point->x, point->y, me->state, me->cost, me->distance);

    struct pf_coord pos, pos_cbk;
    for (unsigned int i = 0; i < ARRAY_SZ(pf_coord_lo_table); i++) {
        pos.x = pf_coord_lo_table[i].x + point->x;
        pos.y = pf_coord_lo_table[i].y + point->y;
        me = pf_get_index(&pos, map);
        
        pos_cbk.x = pos.x +ctx->set.map_start.x;
        pos_cbk.y = pos.y +ctx->set.map_start.y;
        unsigned int cost = pf_get_index(point, map)->cost + ctx->set.pf_traversable_callback(ctx->set.map, &pos_cbk);
        if (cost >= PF_BLOCKED) {
            me->cost = cost;
            me->distance = pf_get_index(point, map)->distance + 1;
            me->state = PF_ENTITY_STATE_CLOSED;
        }
        else if ( (cost < me->cost) || (me->cost == 0) ) {
            me->cost = cost;
            me->distance = pf_get_index(point, map)->distance + 1;
            {//if (me->state == PF_ENTITY_STATE_FREE) {
                me->state = PF_ENTITY_STATE_OPEN;
            }
        }
        //lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf", "p(%d,%d) -> [st: %d/ cst: %d/dst: %d]", pos.x, pos.y, me->state, me->cost, me->distance);
    }

    for (unsigned int i = 0; i < ARRAY_SZ(pf_coord_lo_table); i++) {
        pos.x = pf_coord_lo_table[i].x + point->x;
        pos.y = pf_coord_lo_table[i].y + point->y;
        me = pf_get_index(&pos, map);
        if (me->state == PF_ENTITY_STATE_OPEN) {
            if (pf_flood_map_point(ctx, &pos, target) == true) return true;
        }
    }

    return false;
}

static struct pf_coord get_best_open_node(struct pf_map *map) {
    struct pf_coord cd_best = { .x=PF_BLOCKED, .y=PF_BLOCKED, };
    unsigned int score = PF_BLOCKED;
    unsigned int cost = PF_BLOCKED;

    struct pf_coord point;
    for (point.x = 0; point.x < map->size.x; point.x++) {
        for (point.y = 0; point.y < map->size.y; point.y++) {
            struct pf_map_entity *me = pf_get_index(&point, map);
            if (me->state == PF_ENTITY_STATE_OPEN) {
                if (me->score < score) {
                    cd_best.x = point.x;
                    cd_best.y = point.y;
                    score = me->score;
                    cost = me->cost;
                }
                else if ( (me->score == score) && (me->cost < cost) ){
                    cd_best.x = point.x;
                    cd_best.y = point.y;
                    score = me->score;
                    cost = me->cost;
                }
            }
        }
    }
    return cd_best;
}

static bool pf_astar_loop(struct pf_context *ctx, struct pf_coord *end) {
    if (ctx == NULL) return false;
    if (end == NULL) return false;

    struct pf_map *map = &ctx->map;

    while (true) {
        /* get best node*/
        struct pf_coord point = get_best_open_node(map);
        struct pf_map_entity *me = pf_get_index(&point, map);

        if ( (point.x == PF_BLOCKED) || (point.y == PF_BLOCKED) ) {
            /* failed to find any open node... */
            return false;
        }
        else if ( (point.x == end->x) && (point.y == end->y) ) {
            /* found our goal, yeey! */
            return true;
        }

        //lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf", "entering (%d,%d) -> [st: %d/ cst: %d/dst: %d/scr: %d]", point.x, point.y, me->state, me->cost, me->distance, me->score);

        me->state = PF_ENTITY_STATE_CLOSED;

        /* calculate around current point */
        struct pf_coord pos, pos_cbk;
        for (unsigned int i = 0; i < ARRAY_SZ(pf_coord_lo_table); i++) {
            pos.x = pf_coord_lo_table[i].x + point.x;
            pos.y = pf_coord_lo_table[i].y + point.y;

            if (pos.x >= map->size.x) continue;
            if (pos.y >= map->size.y) continue;
            if (pos.x < 0) continue;
            if (pos.y < 0) continue;
            struct pf_map_entity *me_new = pf_get_index(&pos, map);
            
            pos_cbk.x = pos.x +ctx->set.map_start.x;
            pos_cbk.y = pos.y +ctx->set.map_start.y;
            unsigned int cost = ctx->set.pf_traversable_callback(ctx->set.map, &pos_cbk);

            /* If the new cost is better, OR it was in the free state, update it */
            if ( ( (me->cost +cost) < me_new->cost) || (me_new->state == PF_ENTITY_STATE_FREE) ) {
                me_new->cost = cost + me->cost;
                me_new->score = me_new->cost + (pyth(pos.x - end->x, pos.y - end->y) * 2);
                me_new->distance = me->distance +1;
                me_new->state = PF_ENTITY_STATE_OPEN;
            }

            //lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf", "tested (%d,%d) -> [st: %d/ cst: %d/dst: %d/scr: %d]", pos.x, pos.y, me_new->state, me_new->cost, me_new->distance, me_new->score);
        }
    }

    return false;
}

static bool pf_backtrace(struct pf_map *map, struct pf_coord *end, struct pf_coord coord_lst[], int coord_lst_len) {
    if (map == NULL) return false;
    if (end == NULL) return false;
    if ( (end->x < 0) || (end->x >= map->size.x) ) return false;
    if ( (end->y < 0) || (end->y >= map->size.y) ) return false;

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf","backtrace length %d", coord_lst_len);

    struct pf_coord point = { .x = end->x, .y = end->y, };
    for (int i = 0; i < coord_lst_len; i++) {
        struct pf_map_entity *me = pf_get_index(&point, map);
        if (coord_lst != NULL) {
            coord_lst[i].x = point.x;
            coord_lst[i].y = point.y;
        }

        //lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf", "entering p(%d,%d, [%d] ) -> [st: %d/ cst: %d/dst: %d]", point.x, point.y, i, me->state, me->cost, me->distance);
        if (me->cost >= PF_BLOCKED) return false;
        if (me->distance == 0) return true;

        unsigned int best = PF_BLOCKED;
        struct pf_coord pos, best_pos;
        for (unsigned int j = 0; j < ARRAY_SZ(pf_coord_lo_table); j++) {
            pos.x = pf_coord_lo_table[j].x + point.x;
            pos.y = pf_coord_lo_table[j].y + point.y;
            me = pf_get_index(&pos, map);
            if (me->state != PF_ENTITY_STATE_FREE) {
                if (best > me->cost) {
                    best = me->cost;
                    best_pos.x = pos.x;
                    best_pos.y = pos.y;
                }
            }
            //lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf", "testing p(%d,%d) -> [st: %d/ cst: %d/dst: %d]", pos.x, pos.y, me->state, me->cost, me->distance);
        }

        point.x = best_pos.x;
        point.y = best_pos.y;
    }

    return false;
}

struct pf_context *pf_init(struct pf_settings *pf_set) {
    if (pf_set == NULL) return NULL;
    if (pf_set->pf_traversable_callback == NULL) return NULL;

    struct pf_context *ctx = calloc(1, sizeof(struct pf_context) );
    if (ctx != NULL) {
        memcpy(&ctx->set, pf_set, sizeof(struct pf_settings) );
    }

    return ctx;
}
void pf_exit(struct pf_context *ctx) {
    if (ctx == NULL) return;
    if (ctx->map.map != NULL) free(ctx->map.map);
    free(ctx);
}

bool pf_dijkstra_map(struct pf_context *ctx, struct pf_coord *start) {
    if (ctx == NULL) return false;
    if (ctx->set.pf_traversable_callback == NULL) return false;

    struct pf_map *map = &ctx->map;
    if (map->map != NULL) free(map->map);

    map->size.x = ctx->set.map_end.x - ctx->set.map_start.x;
    map->size.y = ctx->set.map_end.y - ctx->set.map_start.y;
    map->map = calloc(map->size.x * map->size.y, sizeof(struct pf_map_entity) );
    if (map->map == NULL) return false;

    ctx->maximum_distance = pyth(map->size.x, map->size.y);

    pf_get_index(start, map)->cost = 1;
    pf_get_index(start, map)->distance = 0;
    pf_get_index(start, map)->state = PF_ENTITY_STATE_OPEN;
    struct pf_coord dummy = { .x = map->size.x +1, .y = map->size.y +1, };

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf","start at (%d,%d)", start->x,  start->y);
    pf_flood_map_point(ctx, start, &dummy);
    return true;
}

bool pf_calculate_reachability(struct pf_context *ctx) {
    if (ctx == NULL) return false;
    if (ctx->set.pf_traversable_callback == NULL) return false;
    if ( (ctx->set.map_end.x == 0) && (ctx->set.map_end.y == 0) ) return false;

    /* Find a starting point. */
    struct pf_coord target;
    for (int xi = 0; xi < ctx->map.size.x; xi++) {
        for (int yi = 0; yi < ctx->map.size.y; yi++) {
            target.x = xi;
            target.y = yi;
            if (pf_get_index(&target, &ctx->map)->state == PF_ENTITY_STATE_FREE){
                target.x += ctx->set.map_start.x;
                target.y += ctx->set.map_start.y;

                if (ctx->set.pf_traversable_callback(ctx->set.map, &target) < PF_BLOCKED) {
                    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf", "fail at (%d,%d)", target.x,  target.y);
                    return false;
                }
            }
        }
    }
    return true;
}

int pf_astar_map(struct pf_context *ctx, struct pf_coord *start, struct pf_coord *end) {
    if (ctx == NULL) return false;
    if (start == NULL) return false;
    if (end == NULL) return false;
    if (ctx->set.pf_traversable_callback == NULL) return false;
    struct pf_map *map = &ctx->map;

    if (map->map != NULL) free(map->map);

    map->size.x = ctx->set.map_end.x - ctx->set.map_start.x;
    map->size.y = ctx->set.map_end.y - ctx->set.map_start.y;
    map->map = calloc(map->size.x * map->size.y, sizeof(struct pf_map_entity) );
    if (map->map == NULL) return false;

    ctx->maximum_distance = pyth(map->size.x, map->size.y);

    pf_get_index(start, map)->cost = 1;
    pf_get_index(start, map)->distance = 0;
    pf_get_index(start, map)->score = pyth(start->x - end->x, start->y - end->y);
    pf_get_index(start, map)->state = PF_ENTITY_STATE_OPEN;

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf","start at (%d,%d)", start->x,  start->y);
    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf","end at (%d,%d)", end->x,  end->y);

    /* Do astar filling */
    return pf_astar_loop(ctx, end);
}

int pf_calculate_path(struct pf_context *ctx, struct pf_coord *start, struct pf_coord *end, struct pf_coord **coord_lst) {
    int length = -1;
    if (ctx == NULL) return false;
    if (start == NULL) return false;
    if (end == NULL) return false;
    if (ctx->set.pf_traversable_callback == NULL) return false;
    if ( (ctx->set.map_end.x == 0) && (ctx->set.map_end.y == 0) ) return false;
    if (pf_get_index(start, &ctx->map)->state == PF_ENTITY_STATE_FREE) return false;
    if (pf_get_index(end, &ctx->map)->state == PF_ENTITY_STATE_FREE) return false;
    if (pf_get_index(start, &ctx->map)->cost != 1) return false;
    if (pf_get_index(start, &ctx->map)->distance != 0) return false;

    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf","backtrace: start at (%d,%d)", start->x,  start->y);
    lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf","backtrace: end at (%d,%d)", end->x,  end->y);

    length = pf_get_index(end, &ctx->map)->distance+1;

    struct pf_coord *list = NULL;
    if (coord_lst != NULL) {
        *coord_lst = calloc(length +1 , sizeof(struct pf_coord) );
        list = *coord_lst;
    }

    if (pf_backtrace(&ctx->map, end, list, length) == false) {
        lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf","backtrace failed");
        length = -1;
    }
    else lg_printf_l(LG_DEBUG_LEVEL_DEBUG, "pf","backtrace succes");

    if (length == -1) free(list);
    return length;
}

