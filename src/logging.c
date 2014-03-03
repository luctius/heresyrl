#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "logging.h"

struct logging *gbl_log = NULL;

struct logging {
    FILE *log_file;
    enum lg_debug_levels level;
    struct queue *logging_q;
    int logging_q_sz;
    callback_event callback;
    void *priv;
};

struct logging *lg_init(enum lg_debug_levels lvl, int max_size) {
    struct logging *log = malloc(sizeof(struct logging) );
    if (log == NULL) return NULL;

    log->level = lvl;
    log->logging_q_sz = max_size;
    log->logging_q = queue_init_simple(log->logging_q_sz+1);
    log->log_file = fopen("/tmp/heresyrl.log", "w");
    if (log->log_file == NULL) {
        fprintf(stderr, "Could not open logfile\n");
        exit(1);
    }
    log->callback = NULL;

    return log;
}

void lg_exit(struct logging *log) {
    if (log == NULL) return;
    queue_exit(log->logging_q);
    fclose(log->log_file);
    free(log);
}

void lg_set_callback(struct logging *log, void *priv, callback_event ce) {
    if (log == NULL) return;

    log->priv = priv;
    log->callback = ce;
}

struct queue *lg_logging_queue(struct logging *log) {
    return log->logging_q;
}

static void lg_print_to_file(struct logging *log, enum lg_debug_levels dbg_lvl, const char* module, const char* format, va_list args) {
    FILE *fd = stderr;
    char *pre_format = "";

    if (log != NULL)
    {
        if (dbg_lvl > log->level) return;
        fd = log->log_file;
    }

    if (fd == NULL && (dbg_lvl == LG_DEBUG_LEVEL_ERROR || dbg_lvl == LG_DEBUG_LEVEL_WARNING || dbg_lvl == LG_DEBUG_LEVEL_GAME) ) fd = stderr;
    else if (fd == NULL) return;

    if (dbg_lvl >= LG_DEBUG_LEVEL_MAX) dbg_lvl = LG_DEBUG_LEVEL_DEBUG;

    switch (dbg_lvl)
    {
        case LG_DEBUG_LEVEL_GAME:
            pre_format = "[%s" ":Game] ";
            break;
        case LG_DEBUG_LEVEL_DEBUG:
            pre_format = "[%s" ":Debug] ";
            break;
        case LG_DEBUG_LEVEL_INFORMATIONAL:
            pre_format = "[%s" "] ";
            break;
        case LG_DEBUG_LEVEL_WARNING:
            pre_format = "[%s" ":Warning] ";
            break;
        case LG_DEBUG_LEVEL_ERROR:
            pre_format = "[%s" ":Error] ";
            break;
        default:
            "[%s" ":Unknown] ";
            break;
    }

    fprintf(fd, pre_format, module);
    vfprintf(fd, format, args);
    fprintf(fd, "\n");
    fsync(fd);
}

static void lg_print_to_queue(struct logging *log, enum lg_debug_levels dbg_lvl, const char* module, const char* format, va_list args) {
    if (log == NULL) return;
    int tstring_sz = 100;
    char tstring[tstring_sz +1];
    memset(tstring, 0x0, sizeof(tstring));

    struct log_entry *entry = malloc(sizeof(struct log_entry) );
    if (entry != NULL) {
        int len = vsnprintf(tstring, tstring_sz, format, args);

        entry->string = malloc(len +2);
        if (entry->string != NULL) {
            memset(entry->string, 0x0, len+2);
            entry->level = dbg_lvl;
            entry->module = module;
            strncpy(entry->string, tstring, len);
            if (entry->string[len-1] != '\n') entry->string[len] = '\n';
            queue_push_tail(log->logging_q, (intptr_t) entry);

            if (log->callback != NULL) log->callback(log, entry, log->priv);

            while (queue_size(log->logging_q) > log->logging_q_sz) {
                struct log_entry *tmp = (struct log_entry *) queue_pop_head(log->logging_q);
                free(tmp->string);
                free(tmp);
            }
        }
    }
}

void lg_printf_basic(struct logging *log, enum lg_debug_levels dbg_lvl, const char* module, const char* format, va_list args) {

    if ( (log != NULL) && (dbg_lvl > log->level) ) return;
    va_list cpy;
    va_copy(cpy, args);
    lg_print_to_file(log, dbg_lvl, module, format, args);
    lg_print_to_queue(log, dbg_lvl, module, format, cpy);
}

void lg_printf(const char* format, ... ) {
    va_list args;
    va_start(args, format);
    lg_printf_basic(gbl_log, LG_DEBUG_LEVEL_GAME, "Heresyrl", format, args);
    va_end(args);
}

void lg_printf_l(int lvl, const char *module, const char* format, ... ) {
    va_list args;
    va_start(args, format);
    lg_printf_basic(gbl_log, lvl, module , format, args);
    va_end(args);
}

