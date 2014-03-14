#ifndef HERESYRL_DEF_H_
#define HERESYRL_DEF_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "logging.h"

#define container_of(ptr, type, member) ({ \
        typeof( ((type *)0)->member ) *__mptr = (ptr); \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define ARRAY_SZ(a) (sizeof(a) / sizeof(a[0]))
#define bitfield(field, attr) (((field) & attr) > 0)

typedef uint64_t bitfield_t;

/*
   These macros require:
#include "game.h"
#include "dungeon_creator.h"
#include "monster.h"
 */
/* If monster is player, output it, else stay silent. */
#define You(monster, format, args...) if (monster->is_player == true) { lg_printf("%s " format, "You",  ##args); }
/* If monster is player, output it, else stay silent. */
#define Your(monster, format, args...) if (monster->is_player == true) { lg_printf("%s " format, "Your",  ##args); }
/* If monster is player, output it, else stay silent. */
#define You_action(monster, format, args...) if (monster->is_player == true) { lg_printf("%s " format, "You",  ##args); }
/* if monster is a monster, but is on a visible square, output it. */
#define Monster_action(monster, format, args...) if (monster->is_player == false) { if (sd_get_map_me(&monster->pos, gbl_game->current_map)->visible) lg_printf("%s " format, monster->ld_name,  ##args); }

struct gm_game;
struct tl_tile;
struct msr_monster;
struct itm_item;
struct inv_inventory;
struct dc_map_entity;
struct dc_map;
struct random;
struct sgt_sight;
struct pl_player;

enum tile_types;
enum dc_dungeon_type;
enum msr_gender;
enum msr_race;
enum msr_weapon_selection;
enum item_weapon_type;
enum wpn_rof_setting;

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
    DPL_COLOUR_MAX,
};

inline int pyth(int side1, int side2) {
    //return hypot(side1, side2);

    /* 
       This isn't Pythagoras, I know. He would turn around in his grave.
       However we don't really need that, this'll do.
       But we could if we wanted to...
     */
    int a = abs(side1);
    int b = abs(side2);
    return (a > b) ? a : b;
}

#endif /*HERESYRL_DEF_H_*/
