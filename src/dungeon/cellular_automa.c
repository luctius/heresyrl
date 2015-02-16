#include "cellular_automa.h"
#include "coord.h"

static bool ca_round(uint8_t **grid, coord_t *grid_max, uint8_t birth_num, uint8_t surv_num) {
    coord_t p = { .y = 0, .x = 0, };
    for (p.y = 0; p.y < grid_max->y; p.y++) {
        for (p.x = 0; p.x < grid_max->x; p.x++) {
            int num = 0;
            for (int i = 0; i < coord_nhlo_table_sz; i++) {
                coord_t test_p = cd_add(&p, coord_nhlo_table[i]);
                if (cd_within_bound(&test_p, grid_max) ) {
                    if ()
                    num++;
                }
            }
        }
    }
}
