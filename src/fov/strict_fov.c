#include "strict_fov.h"

bool strict_los(struct strict_fov_set *set, coord_t *src, coord_t *dst, bool apply);

void strict_fov(struct strict_fov_set *set, coord_t *src, int radius) {
    coord_t i;
    int radius2 = radius * radius;
    for (i.x = -(int)radius; i.x <= radius; i.x++) { //iterate out of map bounds as well
        for (i.y = -(int)radius; i.y <= radius; i.y++) {
            if(i.x * i.x + i.y * i.y < radius2) {
                coord_t dst = { .x = i.x + src->x, .y = i.y + src->y, };
                strict_los(set, src, &dst, true);
            }
        }
    }
}

bool strict_los(struct strict_fov_set *set, coord_t *src, coord_t *dst, bool apply) {
    int sx, sy, xnext, ynext, dx, dy, x0, y0, x_1, y_1;
    float denom, dist;
    coord_t c;
    
    x0 = src->x;
    y0 = src->y;
    x_1 = dst->x;
    y_1 = dst->y;

    dx = x_1 - x0;
    dy = y_1 - y0;
    if (x0 < x_1) sx = 1;
    else         sx = -1;
    if (y0 < y_1) sy = 1;
    else         sy = -1;

    //if (apply) set->apply(set, src, src);

    // sx and sy are switches that enable us to compute the LOS in a single quarter of x/y plan
    xnext = x0;
    ynext = y0;
    denom = sqrt(dx * dx + dy * dy);
    while (xnext != x_1 || ynext != y_1)
    {
        c.x = xnext;
        c.y = ynext;
        if (set->is_opaque(set, &c, src) == true) {
            if ( (apply) && (set->apply != NULL) ) {
                set->apply(set, &c, src);
            }
            return false;
        }
        // Line-to-point distance formula < 0.5
        if(abs(dy * (xnext - x0 + sx) - dx * (ynext - y0)) / denom < 0.5f)
            xnext += sx;
        else if(abs(dy * (xnext - x0) - dx * (ynext - y0 + sy)) / denom < 0.5f)
            ynext += sy;
        else
        {
            xnext += sx;
            ynext += sy;
        }

        if ( (apply) && (set->apply != NULL) ) {
            if (set->apply(set, &c, src) == false) {
                return false;
            }
        }
    }

    if ( (apply) && (set->apply != NULL) ) {
        if (set->apply(set, &c, src) == false) {
            return false;
        }
    }

    return true;
}
