#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <limits.h>
#include <stdint.h>
#include <stdbool.h>

#define PF_BLOCKED (UINT_MAX / 2)

struct pf_coord {
    int x;
    int y;
};

struct pf_settings {
    int max_traversable_cost;
    struct pf_coord map_start;
    struct pf_coord map_end;

    /* Pointer of local map structure */
    void *map;

    /* Should return the cost of the tile (normally 1 and always higher than zero) 
       or PF_BLOCKED if it is a non-traversable tile.*/
    unsigned int (*pf_traversable_callback)(void *map, struct pf_coord *coord);
};

struct pf_context;

struct pf_context *pf_init(void);
void pf_exit(struct pf_context *ctx);

bool pf_flood_map(struct pf_context *ctx, struct pf_settings *pf_set, struct pf_coord *start);
bool pf_calculate_reachability(struct pf_context *ctx);
int pf_calculate_path(struct pf_context *ctx, struct pf_coord *start, struct pf_coord *end, struct pf_coord **coord_lst);

#endif /* PATHFINDING_H */
