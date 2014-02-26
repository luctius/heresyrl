#pragma once
#ifndef HERESYRL_DEF_H_
#define HERESYRL_DEF_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct tl_tile {
    uint8_t attributes;
    uint8_t type;
    uint8_t colour;
};

struct msr_monster {
    int hp;
    int x_pos;
    int y_pos;
    char icon;
    uint8_t colour;
};

struct sd_map_entity {
    char type;
    bool in_sight;
    bool discovered;
    int light_level;
    struct tl_tile tile;
    struct msr_monster *monster;
};

struct sd_map {
    int x_sz;
    int y_sz;
    struct sd_map_entity map[];
};

enum dpl_colours {
    DPL_COLOUR_NORMAL  =1, /*FG_WHITE,BG_BLACK*/
    DPL_COLOUR_FG_RED,      /* FG_RED,BG_BLACK */
    DPL_COLOUR_FG_GREEN,    /* FG_GREEN,BG_BLACK */
    DPL_COLOUR_FG_YELLOW,   /* FG_YELLOW,BG_BLACK */
    DPL_COLOUR_FG_BLUE,     /* FG_BLUE,BG_BLACK */
    DPL_COLOUR_FG_MAGENTA,  /* FG_MAGENTA,BG_BLACK */
    DPL_COLOUR_FG_CYAN,     /* FG_CYAN,BG_BLACK */

    DPL_COLOUR_FGW_INVERSE, /* BG_WHITE,FG_BLACK */
    DPL_COLOUR_FGW_RED, /* BG_RED,FG_WHITE */
    DPL_COLOUR_FGW_GREEN, /* BG_GREEN,FG_WHITE */
    DPL_COLOUR_FGW_YELLOW, /* BG_YELLOW,FG_WHITE */
    DPL_COLOUR_FGW_BLUE, /* BG_BLUE,FG_WHITE */
    DPL_COLOUR_FGW_MAGENTA, /* BG_MAGENTA,FG_WHITE */
    DPL_COLOUR_FGW_CYAN, /* BG_CYAN,FG_WHITE */

    DPL_COLOUR_BGB_RED, /* BG_RED,FG_BLACK */
    DPL_COLOUR_BGB_GREEN, /* BG_GREEN,FG_BLACK */
    DPL_COLOUR_BGB_YELLOW, /* BG_YELLOW,FG_BLACK */
    DPL_COLOUR_BGB_BLUE, /* BG_BLUE,FG_BLACK */
    DPL_COLOUR_BGB_MAGENTA, /* BG_MAGENTA,FG_BLAC */
    DPL_COLOUR_BGB_CYAN, /* BG_CYAN,FG_BLACK */

    DPL_COLOUR_BGW_RED, /* BG_RED,FG_WHITE */
    DPL_COLOUR_BGW_GREEN, /* BG_GREEN,FG_WHITE */
    DPL_COLOUR_BGW_YELLOW, /* BG_YELLOW,FG_WHITE */
    DPL_COLOUR_BGW_BLUE, /* BG_BLUE,FG_WHITE */
    DPL_COLOUR_BGW_MAGENTA, /* BG_MAGENTA,FG_BLAC */
    DPL_COLOUR_BGW_CYAN, /* BG_CYAN,FG_WHITE */

    DPL_COLOUR_ALL_RED, /* BG_RED,FG_RED */
    DPL_COLOUR_ALL_GREEN, /* BG_GREEN,FG_GREEN */
    DPL_COLOUR_ALL_YELLOW, /* BG_YELLOW,FG_YELLOW */
    DPL_COLOUR_ALL_BLUE, /* BG_BLUE,FG_BLUE */
    DPL_COLOUR_ALL_MAGENTA, /* BG_MAGENTA,FG_MAGENTA */
    DPL_COLOUR_ALL_CYAN, /* BG_CYAN,FG_CYAN */
    DPL_COLOUR_ALL_BLACK, /* BG_CYAN,FG_CYAN */
    DPL_COLOUR_ALL_WHITE, /* BG_CYAN,FG_CYAN */
};

enum lg_debug_levels {
    LG_DEBUG_LEVEL_ERROR,
    LG_DEBUG_LEVEL_WARNING,
    LG_DEBUG_LEVEL_GAME,
    LG_DEBUG_LEVEL_INFORMATIONAL,
    LG_DEBUG_LEVEL_DEBUG,
    LG_DEBUG_LEVEL_MAX,
};

extern struct logging *gbl_log;
void lg_printf(const char* format, ... );
void lg_printf_l(int lvl, const char *module, const char* format, ... );

#endif /*HERESYRL_DEF_H_*/
