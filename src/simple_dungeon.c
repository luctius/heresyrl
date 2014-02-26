#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "simple_dungeon.h"

struct sd_map *sd_alloc_map(int x_sz, int y_sz) {
    if (x_sz < 2) return NULL;
    if (y_sz < 2) return NULL;

    int sz = sizeof(struct sd_map) + ( (x_sz) * y_sz * sizeof(*(((struct sd_map*)0)->map)) ) ;
    struct sd_map *map = malloc(sz);
    if (map == NULL) return NULL;

    map->x_sz = x_sz;
    map->y_sz = y_sz;
    return map;
}

int sd_free_map(struct sd_map *map) {
    if (map == NULL) return EXIT_SUCCESS;
    free(map);
    return EXIT_SUCCESS;
}

int sd_print_map(struct sd_map *map) {
    if (map == NULL) return EXIT_FAILURE;
    if (map->x_sz < 2) return EXIT_FAILURE;
    if (map->y_sz < 2) return EXIT_FAILURE;
    if (map->map == NULL) return EXIT_FAILURE;

    for (int y = 0; y < map->y_sz; y++) {
        for (int x = 0; x < map->x_sz; x++) {
            putchar(SD_GET_INDEX_TYPE(x,y,map));
        }
        putchar('\n');
    }
    putchar('\n');

    return EXIT_SUCCESS;
}

bool sd_tile_instance(struct sd_map *map, enum tile_types tt, int instance, int *xpos, int *ypos) {
    for (int x = 0; x < map->x_sz; x++) {
        for (int y = 0; y < map->y_sz; y++) {
            if (SD_GET_INDEX_TYPE(x,y,map) == tt ) {
                instance--;
                if (instance <= 0) {
                    *xpos = x;
                    *ypos = y;
                    return true;
                }
            }
        }
    }
    return false;
}

int sd_generate_map(struct sd_map *map) {
    if (map == NULL) return EXIT_FAILURE;
    if (map->x_sz < 2) return EXIT_FAILURE;
    if (map->y_sz < 2) return EXIT_FAILURE;
    if (map->map == NULL) return EXIT_FAILURE;

    bool has_stairs_up = false;
    bool has_stairs_down = false;

    for (int x = 0; x < map->x_sz; x++) {
        for (int y = 0; y < map->y_sz; y++) {
            if (y == 0 || y == map->y_sz -1) SD_GET_INDEX(x,y,map).tile = ts_get_tile_type(TILE_TYPE_WALL);
            else if (x == 0 || x == map->x_sz -1) SD_GET_INDEX(x,y,map).tile = ts_get_tile_type(TILE_TYPE_WALL);
            else SD_GET_INDEX(x,y,map).tile = ts_get_tile_type(TILE_TYPE_FLOOR);

            SD_GET_INDEX(x,y,map).in_sight = false;
            SD_GET_INDEX(x,y,map).light_level = false;
            SD_GET_INDEX(x,y,map).monster = NULL;
        }
    }
    while (has_stairs_up == false || has_stairs_down == false) {
        int x = rand() % map->x_sz;
        int y = rand() % map->y_sz;

        if (SD_GET_INDEX_TYPE(x,y,map) == TILE_TYPE_FLOOR ) {
            if (has_stairs_up == false) {
                SD_GET_INDEX(x,y,map).tile = ts_get_tile_type(TILE_TYPE_STAIRS_UP);
                has_stairs_up = true;
            }
            else if (has_stairs_down == false) {
                SD_GET_INDEX(x,y,map).tile = ts_get_tile_type(TILE_TYPE_STAIRS_DOWN);
                has_stairs_down = true;
            }
        }
    }

    return EXIT_SUCCESS;
}

#if FALSE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
 
#define TILE_FLOOR 0
#define TILE_WALL 1
 
 typedef struct {
 	int r1_cutoff, r2_cutoff;
 	int reps;
 } generation_params; 
 
 int **grid;
 int **grid2; 
 
 int fillprob = 40;
 int r1_cutoff = 5, r2_cutoff = 2;
 int size_x = 64, size_y = 20;
 generation_params *params;  
 
 generation_params *params_set;
 int generations;
 
 int randpick(void)
 {
 	if(rand()%100 < fillprob)
 		return TILE_WALL;
 	else
 		return TILE_FLOOR;
 }
 
 void initmap(void)
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
		grid2[yi][xi] = TILE_WALL;
 
	for(yi=0; yi<size_y; yi++)
		grid[yi][0] = grid[yi][size_x-1] = TILE_WALL;
	for(xi=0; xi<size_x; xi++)
		grid[0][xi] = grid[size_y-1][xi] = TILE_WALL;
 }
 
 void generation(void)
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
 			if(grid[yi+ii][xi+jj] != TILE_FLOOR)
 				adjcount_r1++;
 		}
 		for(ii=yi-2; ii<=yi+2; ii++)
 		for(jj=xi-2; jj<=xi+2; jj++)
 		{
 			if(abs(ii-yi)==2 && abs(jj-xi)==2)
 				continue;
 			if(ii<0 || jj<0 || ii>=size_y || jj>=size_x)
 				continue;
 			if(grid[ii][jj] != TILE_FLOOR)
 				adjcount_r2++;
 		}
 		if(adjcount_r1 >= params->r1_cutoff || adjcount_r2 <= params->r2_cutoff)
 			grid2[yi][xi] = TILE_WALL;
 		else
 			grid2[yi][xi] = TILE_FLOOR;
 	}
 	for(yi=1; yi<size_y-1; yi++)
 	for(xi=1; xi<size_x-1; xi++)
 		grid[yi][xi] = grid2[yi][xi];
 } 
 
 void printfunc(void)
 {
 	int ii;
 
 	printf("W[0](p) = rand[0,100) < %i\n", fillprob);
 
 	for(ii=0; ii<generations; ii++)
 	{
 		printf("Repeat %i: W'(p) = R[1](p) >= %i",
 			params_set[ii].reps, params_set[ii].r1_cutoff);
 
 		if(params_set[ii].r2_cutoff >= 0)
 			printf(" || R[2](p) <= %i\n", params_set[ii].r2_cutoff);
 		else
 			putchar('\n');
 	}
 }
 
 void printmap(void)
 {
 	int xi, yi;
 
 	for(yi=0; yi<size_y; yi++)
 	{
 		for(xi=0; xi<size_x; xi++)
 		{
 			switch(grid[yi][xi]) {
 				case TILE_WALL:  putchar('#'); break;
 				case TILE_FLOOR: putchar('.'); break;
 			}
 		}
 		putchar('\n');
 	}
 }
 
 int main(int argc, char **argv)
 {
 	int ii, jj;
 
 	if(argc < 7) {
 		printf("Usage: %s xsize ysize fill (r1 r2 count)+\n", argv[0]);
 		return 1;
 	}
 	size_x     = atoi(argv[1]);
 	size_y     = atoi(argv[2]);
 	fillprob   = atoi(argv[3]);
 
 	generations = (argc-4)/3;
 
 	params = params_set = (generation_params*)malloc( sizeof(generation_params) * generations );
 
 	for(ii=4; ii+2<argc; ii+=3)
 	{
 		params->r1_cutoff  = atoi(argv[ii]);
 		params->r2_cutoff  = atoi(argv[ii+1]);
 		params->reps = atoi(argv[ii+2]);
 		params++;
 	}
 
 	srand(time(NULL));
 
 	initmap();
 
 	for(ii=0; ii<generations; ii++)
 	{
 		params = &params_set[ii];
 		for(jj=0; jj<params->reps; jj++)
 			generation();
 	}
 	printfunc();
 	printmap();
 	return 0;
 }
#endif
