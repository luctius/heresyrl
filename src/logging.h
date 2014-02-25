#pragma once
#ifndef LOGGING_H_
#define LOGGING_H_

#include "queue.h"

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

struct log_entry {
    char *module;
    char *string;
    enum lg_debug_levels level;
};

typedef void (*callback_event)(struct logging *log, struct log_entry *entry, void *priv);

struct logging *lg_init(enum lg_debug_levels lvl, int max_size);
void lg_exit(struct logging *log);
void lg_set_callback(struct logging *log, void *priv, callback_event ce);
void lg_change_debug_lvl(struct logging *log, enum lg_debug_levels lvl);

struct queue *lg_logging_queue(struct logging *log);

void lg_printf(const char* format, ... );
void lg_printf_l(struct logging *log, int lvl, const char *module, const char* format, ... );
void lg_printf_basic(struct logging *log, enum lg_debug_levels dbg_lvl, const char* module, const char* format, va_list args);

#endif /*LOGGING_H_*/
