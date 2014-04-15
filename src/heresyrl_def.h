#ifndef HERESYRL_DEF_H_
#define HERESYRL_DEF_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

#include "logging.h"

#ifndef FIX_UNUSED
#define FIX_UNUSED(X) (void) (X) /* avoid warnings for unused params */
#endif

#define container_of(ptr, type, member) ({ \
        typeof( ((type *)0)->member ) *__mptr = (ptr); \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define ARRAY_SZ(a) (sizeof(a) / sizeof(a[0]))

#define bf(x) (1<<x)
#define set_bf(field, attr) (field |= bf(attr) )
#define clr_bf(field, attr) (field &= ~(bf(attr) ) )
#define test_bf(field, attr) ( (field & bf(attr) ) > 0)
typedef uint_least64_t bitfield_t;
#define bitfield_width (sizeof(bitfield_t) * CHAR_BIT)

struct gm_game;
struct tl_tile;
struct msr_monster;
struct itm_item;
struct inv_inventory;
struct dm_map_entity;
struct dm_map;
struct random;
struct sgt_sight;
struct pl_player;
struct input;
struct cdn_condition_list;
struct cdn_condition;

enum tile_types;
enum dm_dungeon_type;
enum msr_gender;
enum msr_race;
enum msr_weapon_selection;
enum item_weapon_type;
enum wpn_rof_setting;

struct pl_player {
    struct msr_monster *player;
    int xp_current;
    int xp_spend;

    struct pf_context *player_map;
    coord_t player_map_pos;
};

struct gm_game {
    unsigned long initial_seed;
    struct random *random;

    struct sgt_sight *sight;
    struct inp_input *input;

    struct dm_map *current_map;
    struct pl_player player_data;
    int turn;
    bool running;
};

extern struct gm_game *gbl_game;

extern int get_colour(int cc);
enum term_colours {
    TERM_COLOUR_DARK,
    TERM_COLOUR_WHITE,
    TERM_COLOUR_SLATE,
    TERM_COLOUR_ORANGE,
    TERM_COLOUR_RED,
    TERM_COLOUR_GREEN,
    TERM_COLOUR_BLUE,
    TERM_COLOUR_UMBER,
    TERM_COLOUR_L_DARK,
    TERM_COLOUR_L_WHITE,
    TERM_COLOUR_L_PURPLE,
    TERM_COLOUR_YELLOW,
    TERM_COLOUR_L_RED,
    TERM_COLOUR_L_GREEN,
    TERM_COLOUR_L_BLUE,
    TERM_COLOUR_L_UMBER,

    TERM_COLOUR_PURPLE,
    TERM_COLOUR_VIOLET,
    TERM_COLOUR_TEAL,
    TERM_COLOUR_MUD,
    TERM_COLOUR_L_YELLOW,
    TERM_COLOUR_MAGENTA,
    TERM_COLOUR_L_TEAL,
    TERM_COLOUR_L_VIOLET,
    TERM_COLOUR_L_PINK,
    TERM_COLOUR_MUSTARD,
    TERM_COLOUR_BLUE_SLATE,
    TERM_COLOUR_DEEP_L_BLUE,

    TERM_COLOUR_BG_WHITE,
    TERM_COLOUR_BG_SLATE,
    TERM_COLOUR_BG_ORANGE,
    TERM_COLOUR_BG_RED,
    TERM_COLOUR_BG_GREEN,
    TERM_COLOUR_BG_BLUE,
    TERM_COLOUR_BG_UMBER,
    TERM_COLOUR_BG_L_WHITE,
    TERM_COLOUR_BG_L_PURPLE,
    TERM_COLOUR_BG_YELLOW,
    TERM_COLOUR_BG_L_RED,
    TERM_COLOUR_BG_L_GREEN,
    TERM_COLOUR_BG_L_BLUE,
    TERM_COLOUR_BG_L_UMBER,

    TERM_COLOUR_BG_PURPLE,
    TERM_COLOUR_BG_VIOLET,
    TERM_COLOUR_BG_TEAL,
    TERM_COLOUR_BG_MUD,
    TERM_COLOUR_BG_L_YELLOW,
    TERM_COLOUR_BG_MAGENTA,
    TERM_COLOUR_BG_L_TEAL,
    TERM_COLOUR_BG_L_VIOLET,
    TERM_COLOUR_BG_L_PINK,
    TERM_COLOUR_BG_MUSTARD,
    TERM_COLOUR_BG_BLUE_SLATE,
    TERM_COLOUR_BG_DEEP_L_BLUE,
    TERM_COLOUR_MAX,
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

void generate_colours(void);

#endif /*HERESYRL_DEF_H_*/
