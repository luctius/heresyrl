#include <math.h>
#include <sys/param.h>
#include "fight.h"

int fght_calc_lof_path(coord_t *s, coord_t *e, coord_t path_list[], int path_list_sz) {
    int length = cd_pyth(s, e);
    float px = s->x;
    float py = s->y;

    int i = 0;
    for (i = 0; i < MIN(path_list_sz, length); i++) {
        px += ( (e->x - px) / (length - i) );
        py += ( (e->y - py) / (length - i) );
        path_list[i].x = round(px);
        path_list[i].y = round(py);
    }
    if ( (i+1) < path_list_sz) {
        path_list[i+1].x = e->x;
        path_list[i+1].y = e->y;
        return (i+1);
    }
    return MIN(path_list_sz, length);
}
