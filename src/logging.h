#ifndef LOGGING_H_
#define LOGGING_H_

#include <stdarg.h>

#include "queue.h"
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
    LG_DEBUG_LEVEL_INFORMATIONAL,
    LG_DEBUG_LEVEL_DEBUG,
    LG_DEBUG_LEVEL_MAX,
};

enum lg_channel {
    LG_CHANNEL_DEBUG,
    LG_CHANNEL_WARNING,
    LG_CHANNEL_PLAIN,
    LG_CHANNEL_GM,
    LG_CHANNEL_SYSTEM,
    LG_CHANNEL_SAY,
    LG_CHANNEL_NUMBER,
    LG_CHANNEL_MAX,
};

struct log_entry {
    int turn;
    int repeat;

    const char *module;
    
    enum lg_debug_levels level;
    enum lg_channel channel;
    char *string;
};

typedef void (*callback_event)(struct logging *log, struct log_entry *entry, void *priv);
struct queue *lg_queue(struct logging *log);

struct logging *lg_init(char *logfile, enum lg_debug_levels lvl, int max_size);
void lg_exit(struct logging *log);
void lg_set_debug_lvl(struct logging *log, enum lg_debug_levels lvl);
void lg_set_callback(struct logging *log, void *priv, callback_event ce);

#define msg(f, a...) lg_printf_l(LG_DEBUG_LEVEL_GAME, __FILE__, f, ##a)
#define lg_print(f, a...) lg_printf_l(LG_DEBUG_LEVEL_INFORMATIONAL, __FILE__, f, ##a)
#define lg_debug(f, a...) lg_printf_l(LG_DEBUG_LEVEL_DEBUG, __FILE__, f, ##a)
#define lg_warning(f, a...) lg_printf_l(LG_DEBUG_LEVEL_WARNING, __FILE__, f, ##a)
#define lg_error(f, a...) lg_printf_l(LG_DEBUG_LEVEL_ERROR, __FILE__, f, ##a)
void lg_printf_l(int lvl, const char *module, const char* format, ... );
void lg_printf_basic(struct logging *log, enum lg_debug_levels dbg_lvl, const char *module, const char *format, va_list args);

void msg_internal(coord_t *origin, coord_t *target, enum lg_channel c, const char *format, ...);
#define GM_msg(f, a...)            do                     { msg_internal(NULL,NULL,    LG_CHANNEL_GM, f, ##a); } while (0)
#define System_msg(f, a...)        do                     { msg_internal(NULL,NULL,    LG_CHANNEL_SYSTEM, f, ##a); } while (0)
#define Event_msg(pos, f, a...)    do                     { msg_internal(&pos,NULL,    LG_CHANNEL_PLAIN, f, ##a); } while (0)
#define You(m, f, a...)            do {if (m->is_player)  { msg_internal(&m->pos,NULL, LG_CHANNEL_PLAIN, "You " f, ##a); } } while (0)
#define Your(m, f, a...)           do {if (m->is_player)  { msg_internal(&m->pos,NULL, LG_CHANNEL_PLAIN, "Your " f, ##a); } } while (0)
#define You_msg(m, f, a...)        do {if (m->is_player)  { msg_internal(&m->pos,NULL, LG_CHANNEL_PLAIN, f, ##a); } } while (0)
#define Monster(m, f, a...)        do {if (!m->is_player) { msg_internal(&m->pos,NULL, LG_CHANNEL_PLAIN, "%s " f, msr_ldname(m), ##a); } } while (0)
#define Monster_he(m, f, a...)     do {if (!m->is_player) { msg_internal(&m->pos,NULL, LG_CHANNEL_PLAIN, "%s " f, msr_gender_name(m, false), ##a); } } while (0)
#define Monster_his(m, f, a...)    do {if (!m->is_player) { msg_internal(&m->pos,NULL, LG_CHANNEL_PLAIN, "%s " f, msr_gender_name(m, true), ##a); } } while (0)
#define Monster_msg(m, f, a...)    do {if (!m->is_player) { msg_internal(&m->pos,NULL, LG_CHANNEL_PLAIN, f, ##a); } } while (0)

#endif /*LOGGING_H_*/
