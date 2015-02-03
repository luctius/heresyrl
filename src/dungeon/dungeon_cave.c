#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
 
#include "dungeon_cave.h"
#include "dungeon_map.h"
#include "tiles.h"
#include "random.h"
#include "coord.h"

struct generation_params {
    int r1_cutoff, r2_cutoff;
    int reps;
}; 
 
static int **grid;
static int **grid2; 
 
static unsigned int fillprob = 40;
static int size_x = 64, size_y = 20;
static struct generation_params *params;  
 
static struct generation_params *params_set;
static int generations;

static struct random *cave_random = NULL;
 
static int randpick(void)
{
    if(random_int32(cave_random)%100 < fillprob) {
        return TILE_ID_CONCRETE_WALL;
    }
    return TILE_ID_CONCRETE_FLOOR;
}
 
static void initmap(void)
{
    int xi, yi;

    grid  = (int**)malloc(sizeof(int*) * size_y);
    grid2 = (int**)malloc(sizeof(int*) * size_y);

    for(yi=0; yi<size_y; yi++)
    {
        grid [yi] = (int*)malloc(sizeof(int) * size_x);
        grid2[yi] = (int*)malloc(sizeof(int) * size_x);
    }

    for(yi=1; yi<size_y-1; yi++)
        for(xi=1; xi<size_x-1; xi++)
            grid[yi][xi] = randpick();

    for(yi=0; yi<size_y; yi++)
        for(xi=0; xi<size_x; xi++)
            grid2[yi][xi] = TILE_ID_CONCRETE_WALL;

    for(yi=0; yi<size_y; yi++)
        grid[yi][0] = grid[yi][size_x-1] = TILE_ID_CONCRETE_WALL;
    for(xi=0; xi<size_x; xi++)
        grid[0][xi] = grid[size_y-1][xi] = TILE_ID_CONCRETE_WALL;
}

static void exitmap(void)
{
    int yi;

    for(yi=0; yi<size_y; yi++)
    {
        free(grid[yi]);
        free(grid2[yi]);
    }
    free(grid);
    free(grid2);
}
 
static void generation(void)
{
    int xi, yi, ii, jj;

    for(yi=1; yi<size_y-1; yi++)
        for(xi=1; xi<size_x-1; xi++)
        {
            int adjcount_r1 = 0,
            adjcount_r2 = 0;

            for(ii=-1; ii<=1; ii++)
                for(jj=-1; jj<=1; jj++)
                {
                    if(grid[yi+ii][xi+jj] != TILE_ID_CONCRETE_FLOOR)
                        adjcount_r1++;
                }
            for(ii=yi-2; ii<=yi+2; ii++)
                for(jj=xi-2; jj<=xi+2; jj++)
                {
                    if(abs(ii-yi)==2 && abs(jj-xi)==2)
                        continue;
                    if(ii<0 || jj<0 || ii>=size_y || jj>=size_x)
                        continue;
                    if(grid[ii][jj] != TILE_ID_CONCRETE_FLOOR)
                        adjcount_r2++;
                }
            if(adjcount_r1 >= params->r1_cutoff || adjcount_r2 <= params->r2_cutoff)
                grid2[yi][xi] = TILE_ID_CONCRETE_WALL;
            else
                grid2[yi][xi] = TILE_ID_CONCRETE_FLOOR;
        }
    for(yi=1; yi<size_y-1; yi++)
        for(xi=1; xi<size_x-1; xi++)
            grid[yi][xi] = grid2[yi][xi];
} 

bool cave_generate_map(struct dm_map *map, struct random *r, enum dm_dungeon_type type, coord_t *ul, coord_t *dr) {
    int ii, jj, yi, xi;

    if (type != DUNGEON_TYPE_CAVE) return -1;

    size_x     = dr->x - ul->x;
    size_y     = dr->y - ul->y;
    fillprob   = 45;

    cave_random = r;

    generations = 3;
    params = params_set = malloc( sizeof(struct generation_params) * generations );
    assert(params != NULL);

    //uint8_t arg_array[] = {5, 1, 6, 4, 2, 3, 5, 1, 6}; /* Should be 3 * generations */
    uint8_t arg_array[] = {5, 1, 6, 4, 1, 4, 6, 1, 6}; /* Should be 3 * generations */
    assert( (int) ARRAY_SZ(arg_array) >= generations * 3);

    for(ii=0; ii < (generations *3); ii+=3)
    {
        params->r1_cutoff  = arg_array[ii];
        params->r2_cutoff  = arg_array[ii+1];
        params->reps = arg_array[ii+2];
        params++;
    }

    initmap();

    for(ii=0; ii<generations; ii++)
    {
        params = &params_set[ii];
        for(jj=0; jj<params->reps; jj++)
            generation();
    }

    for(yi=0; yi<size_y; yi++)
    {
        for(xi=0; xi<size_x; xi++)
        {
            coord_t c = cd_create(xi + ul->x,yi + ul->y);
            if (grid[yi][xi] == TILE_ID_CONCRETE_WALL) {
                if(random_int32(cave_random)%100 < 1) {
                    grid[yi][xi] = TILE_ID_CONCRETE_WALL_LIT;
                }
            }

            dm_get_map_me(&c,map)->tile = ts_get_tile_specific(grid[yi][xi]);
        }
    }
    free(params_set);

    exitmap();
    return 0;
}
