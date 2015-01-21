#ifndef LOGGING_H_
#define LOGGING_H_

#include <stdarg.h>

#include "cqc.h"
#include "coord.h"

struct msr_monster;
struct itm_item;

#define LG_COLOUR_NORMAL          ""
#define LG_COLOUR_RESET           "\033[m"
#define LG_COLOUR_BOLD            "\033[1m"
#define LG_COLOUR_RED             "\033[31m"
#define LG_COLOUR_GREEN           "\033[32m"
#define LG_COLOUR_YELLOW          "\033[33m"
#define LG_COLOUR_BLUE            "\033[34m"
#define LG_COLOUR_MAGENTA         "\033[35m"
#define LG_COLOUR_CYAN            "\033[36m"
#define LG_COLOUR_BOLD_RED        "\033[1;31m"
#define LG_COLOUR_BOLD_GREEN      "\033[1;32m"
#define LG_COLOUR_BOLD_YELLOW     "\033[1;33m"
#define LG_COLOUR_BOLD_BLUE       "\033[1;34m"
#define LG_COLOUR_BOLD_MAGENTA    "\033[1;35m"
#define LG_COLOUR_BOLD_CYAN       "\033[1;36m"
#define LG_COLOUR_BG_RED          "\033[41m"
#define LG_COLOUR_BG_GREEN        "\033[42m"
#define LG_COLOUR_BG_YELLOW       "\033[43m"
#define LG_COLOUR_BG_BLUE         "\033[44m"
#define LG_COLOUR_BG_MAGENTA      "\033[45m"
#define LG_COLOUR_BG_CYAN         "\033[46m"

struct logging;
extern struct logging *gbl_log;

enum lg_debug_levels {
    LG_DEBUG_LEVEL_ERROR,
    LG_DEBUG_LEVEL_WARNING,
    LG_DEBUG_LEVEL_GAME,
    LG_DEBUG_LEVEL_GAME_INFO,
    LG_DEBUG_LEVEL_INFORMATIONAL,
    LG_DEBUG_LEVEL_DEBUG,
    LG_DEBUG_LEVEL_MAX,
};

struct log_entry {
    int turn;
    int repeat;

    char *module;
    int line;
    
    enum lg_debug_levels level;
    char *string;
};


/*
Warning: Stacking multiple of the same cs_XXX is not supported yet
*/
#define cs_MONSTER  "<monster>"
#define cs_PLAYER   "<player>"
#define cs_DAMAGE   "<dmg>"
#define cs_WARNING  "<warning>"
#define cs_CRITICAL "<critical>"
#define cs_GM       "<gm>"
#define cs_SYSTEM   "<system>"
#define cs_ATTR     "<attr>"
#define cs_OLD      "<old>"
int clrstr_to_attr(const char *);

typedef void (*callback_event)(struct logging *log, struct log_entry *entry, void *priv);


struct logging *lg_init(char *logfile, enum lg_debug_levels lvl, uint32_t max_size);
void lg_exit(struct logging *log);
void lg_set_debug_lvl(struct logging *log, enum lg_debug_levels lvl);
void lg_set_callback(struct logging *log, void *priv, callback_event ce);

int lg_size(struct logging *log);
struct log_entry *lg_peek(struct logging *log, int idx);
void lg_add_entry(struct logging *log_ctx, struct log_entry *le_given);

#define msg(f, a...)        lg_printf_l(LG_DEBUG_LEVEL_GAME,          __FILE__, __LINE__, f, ##a)
#define lg_print(f, a...)   lg_printf_l(LG_DEBUG_LEVEL_INFORMATIONAL, __FILE__, __LINE__, f, ##a)
#define lg_debug(f, a...)   lg_printf_l(LG_DEBUG_LEVEL_DEBUG,         __FILE__, __LINE__, f, ##a)
#define lg_warning(f, a...) lg_printf_l(LG_DEBUG_LEVEL_WARNING,       __FILE__, __LINE__, f, ##a)
#define lg_error(f, a...)   lg_printf_l(LG_DEBUG_LEVEL_ERROR,         __FILE__, __LINE__, f, ##a)

#define GM_msg(f, a...)                 do                     { msg_internal(NULL,NULL,        __FILE__, __LINE__, cs_GM f cs_GM, ##a); } while (0)
#define System_msg(f, a...)             do                     { msg_internal(NULL,NULL,        __FILE__, __LINE__, cs_SYSTEM f cs_SYSTEM, ##a); } while (0)
#define Warning(f, a...)                do                     { msg_internal(NULL,NULL,        __FILE__, __LINE__, cs_CRITICAL "**" "Warning, " f "**" cs_CRITICAL, ##a); } while (0)
#define Info(f, a...)                   do                     { msg_internal(NULL,NULL,        __FILE__, __LINE__, f, ##a); } while (0)
#define Event_msg(p1, f, a...)          do                     { msg_internal(p1,NULL,          __FILE__, __LINE__, f, ##a); } while (0)
#define Event_tgt_msg(p1,p2, f, a...)   do                     { msg_internal(p1,p2,            __FILE__, __LINE__, f, ##a); } while (0)
#define You(m, f, a...)                 do {if (m->is_player)  { msg_internal(&m->pos,NULL,     __FILE__, __LINE__, cs_PLAYER "You" cs_PLAYER " " f, ##a); } } while (0)
#define Your(m, f, a...)                do {if (m->is_player)  { msg_internal(&m->pos,NULL,     __FILE__, __LINE__, cs_PLAYER "Your" cs_PLAYER " " f, ##a); } } while (0)
#define You_msg(m, f, a...)             do {if (m->is_player)  { msg_internal(&m->pos,NULL,     __FILE__, __LINE__, f, ##a); } } while (0)
#define Monster(m, f, a...)             do {if (!m->is_player) { msg_internal(&m->pos,NULL,     __FILE__, __LINE__, "%s " f, msr_ldname(m), ##a); } } while (0)
#define Monster_he(m, f, a...)          do {if (!m->is_player) { msg_internal(&m->pos,NULL,     __FILE__, __LINE__, "%s " f, msr_gender_name(m, false), ##a); } } while (0)
#define Monster_his(m, f, a...)         do {if (!m->is_player) { msg_internal(&m->pos,NULL,     __FILE__, __LINE__, "%s " f, msr_gender_name(m, true), ##a); } } while (0)
#define Monster_msg(m, f, a...)         do {if (!m->is_player) { msg_internal(&m->pos,NULL,     __FILE__, __LINE__, f, ##a); } } while (0)

#define Monster_tgt(m, m2, f, a...)     do {if (!m->is_player) { msg_internal(&m->pos,&m2->pos, __FILE__, __LINE__, "%s " f, msr_ldname(m), ##a); } } while (0)
#define Monster_tgt_he(m, m2, f, a...)  do {if (!m->is_player) { msg_internal(&m->pos,&m2->pos, __FILE__, __LINE__, "%s " f, msr_gender_name(m, false), ##a); } } while (0)
#define Monster_tgt_his(m, m2, f, a...) do {if (!m->is_player) { msg_internal(&m->pos,&m2->pos, __FILE__, __LINE__, "%s " f, msr_gender_name(m, true), ##a); } } while (0)
#define Monster_tgt_msg(m, m2, f, a...) do {if (!m->is_player) { msg_internal(&m->pos,&m2->pos, __FILE__, __LINE__, f, ##a); } } while (0)

/*Do not use directly*/
void lg_printf_l(int lvl, const char *module, int line, const char* format, ... );
void lg_printf_basic(struct logging *log, enum lg_debug_levels dbg_lvl, const char *module, int line, const char *format, va_list args);
void msg_internal(coord_t *origin, coord_t *target, const char* module, int line, const char *format, ...);

#endif /*LOGGING_H_*/
