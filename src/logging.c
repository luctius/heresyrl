#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "logging.h"
#include "game.h"
#include "monster/monster.h"
#include "dungeon/dungeon_map.h"

struct logging *gbl_log = NULL;

struct logging {
    FILE *log_file;

    enum lg_debug_levels level;
    struct queue *logging_q;

    int logging_q_sz;

    callback_event callback;
    void *priv;

    struct log_entry *log_last;
};

static void le_free(struct log_entry *e) {
    free(e->string);
    free(e);
}

struct logging *lg_init(char *logfile, enum lg_debug_levels lvl, int max_size) {
    struct logging *log_ctx = calloc(1, sizeof(struct logging) );
    if (log_ctx == NULL) return NULL;

    log_ctx->level = lvl;
    log_ctx->logging_q_sz = max_size;
    log_ctx->logging_q = queue_init_simple(log_ctx->logging_q_sz+1);
    log_ctx->log_file = fopen(logfile, "w");
    if (log_ctx->log_file == NULL) {
        fprintf(stderr, "Could not open logfile %s\n", logfile);
        exit(1);
    }
    log_ctx->callback = NULL;

    return log_ctx;
}

void lg_exit(struct logging *log_ctx) {
    if (log_ctx == NULL) return;

    while (queue_size(log_ctx->logging_q) > 0) {
        struct log_entry *tmp = queue_pop_head(log_ctx->logging_q).vp;
        le_free(tmp);
    }

    queue_exit(log_ctx->logging_q);
    fclose(log_ctx->log_file);
    free(log_ctx);
}

void lg_set_callback(struct logging *log_ctx, void *priv, callback_event ce) {
    if (log_ctx == NULL) return;

    log_ctx->priv = priv;
    log_ctx->callback = ce;
}

struct queue *lg_queue(struct logging *log_ctx) {
    return log_ctx->logging_q;
}

static bool le_is_equal(struct log_entry *a, struct log_entry *b) {
    if (a == NULL) return false;
    if (b == NULL) return false;

    if (a->level != b->level)   return false;
    if (a->module != b->module) return false;

    if (a->channel != b->channel) return false;
    if (a->string == NULL) return false;
    if (b->string == NULL) return false;
    if (strcmp(a->string, b->string) != 0) return false;

    return true;
}

static void lg_print_to_file(struct logging *log_ctx, struct log_entry *entry) {
    FILE *fd = stderr;
    const char *pre_format = "";

    if (log_ctx != NULL)
    {
        if (entry->level > log_ctx->level) return;
        fd = log_ctx->log_file;
    }

    if (fd == NULL && (entry->level == LG_DEBUG_LEVEL_ERROR || entry->level == LG_DEBUG_LEVEL_WARNING || entry->level == LG_DEBUG_LEVEL_GAME) ) fd = stderr;
    else if (fd == NULL) return;

    if (entry->level >= LG_DEBUG_LEVEL_MAX) entry->level = LG_DEBUG_LEVEL_DEBUG;

    switch (entry->level)
    {
        case LG_DEBUG_LEVEL_GAME:
            pre_format = "Game";
            break;
        case LG_DEBUG_LEVEL_DEBUG:
            pre_format = "Debug";
            break;
        case LG_DEBUG_LEVEL_INFORMATIONAL:
            pre_format = "Info";
            break;
        case LG_DEBUG_LEVEL_WARNING:
            pre_format = "Warning";
            break;
        case LG_DEBUG_LEVEL_ERROR:
            pre_format = "Error";
            break;
        default:
            pre_format = "Unknown";
            break;
    }

    fprintf(fd, "[%s:%s][%d] ", entry->module, pre_format, entry->turn);
    fprintf(fd, "%s", entry->string);

    if (entry->repeat > 1) fprintf(fd, " x%d", entry->repeat);
    fprintf(fd, "\n");
    fflush(fd);
}

static void lg_print_to_queue(struct logging *log_ctx, struct log_entry *entry) {
    if (log_ctx == NULL) return;
    //if (dbg_lvl > LG_DEBUG_LEVEL_GAME) return;

    union qe e;
    e.vp = entry;
    queue_push_tail(log_ctx->logging_q, e);

    if (log_ctx->callback != NULL) log_ctx->callback(log_ctx, entry, log_ctx->priv);

    while (queue_size(log_ctx->logging_q) > log_ctx->logging_q_sz) {
        struct log_entry *tmp = (struct log_entry *) queue_pop_head(log_ctx->logging_q).vp;
        le_free(tmp);
    }
}

#define STRING_MAX 500
void lg_printf_basic(struct logging *log_ctx, enum lg_debug_levels dbg_lvl, const char* module, const char* format, va_list args) {
    if ( (log_ctx != NULL) && (dbg_lvl > log_ctx->level) ) return;
    struct log_entry *le = calloc(1, sizeof(struct log_entry) );
    if (le == NULL) return;

    le->level = dbg_lvl;
    le->repeat = 0;
    le->module = module;
    le->turn = 0;
    if (gbl_game != NULL) le->turn = gbl_game->turn;


    le->channel = LG_CHANNEL_DEBUG;
    if ( (dbg_lvl == LG_DEBUG_LEVEL_ERROR) || (dbg_lvl == LG_DEBUG_LEVEL_WARNING) ) {
        le->channel = LG_CHANNEL_WARNING;
    }
    
    le->string = calloc(STRING_MAX, sizeof(char) );
    vsnprintf(le->string, STRING_MAX, format, args);
    le->string = realloc(le->string, strlen(le->string) +1);

    lg_print_to_file(log_ctx, le);
    lg_print_to_queue(log_ctx, le);
}

void lg_printf_l(int lvl, const char *module, const char* format, ... ) {
    va_list args;
    va_start(args, format);
    lg_printf_basic(gbl_log, lvl, module , format, args);
    va_end(args);
}

bool msg_valid(coord_t *origin, coord_t *target) {
    struct dm_map_entity *me;

    /* if all coords are NULL, accept it as a system msg */
    if ( (origin == NULL) && (target == NULL) ) return true;

    if (gbl_game->current_map == NULL) return false;
    if (dm_verify_map(gbl_game->current_map) == false) return false;

    if (origin != NULL) {
        me = dm_get_map_me(origin, gbl_game->current_map);
        if (me == NULL) return false;

        /* 
           this message system has two channels.
           One for the player and one for monsters.

           if origin is a player, accept the player channel.
           if the player is otherwise involved, accept it in
           the monster channel. otherwise discard the messages.
         */

        /* if origin is the player, accept it*/
        if (me->monster->is_player == true) return true;

        /* if the origin is visible by the player, accept it */
        if (me->visible == true)  return true;
    }

    /* origin does not effect the playe, and the target is invalid, do not accept */
    if (target == NULL) return false;

    me = dm_get_map_me(target, gbl_game->current_map);
    if (me == NULL) return false;

    /* if the target is visible (may because I'm on it), accept it.*/
    if (me->visible == true) return true;
    return false;
}

void msg_internal(coord_t *origin, coord_t *target, enum lg_channel c, const char *format, ...) {
    if (gbl_log == NULL) return;


    /* check if the message is relevant. */
    if (msg_valid(origin, target) == false) return;

    /* Message is accepted, either because it is a system message, 
       or because either the origin or target are of interrest to the 
       player. 
    */

    struct log_entry *le = malloc(sizeof(struct log_entry) );
    le->turn    = gbl_game->turn;
    le->repeat  = 1;
    le->module  = "game";
    le->channel = c;
    le->level   = LG_DEBUG_LEVEL_GAME;

    /* allocate enough memory for the string to be copied */
    le->string = calloc(STRING_MAX, sizeof(char) );

    /* copy the string from the vararg list */
    va_list args;
    va_start(args, format);
    vsnprintf(le->string, STRING_MAX, format, args);
    va_end(args);

    /* shrink string to minimum required. */
    le->string = realloc(le->string, strlen(le->string) +1);

    if (le_is_equal(le, gbl_log->log_last) ) {
        gbl_log->log_last->repeat++;
        gbl_log->log_last->turn = gbl_game->turn;

        /* There is no new entry, so we have to force an update ourselves */
        if (gbl_log->callback != NULL) gbl_log->callback(gbl_log, gbl_log->log_last, gbl_log->priv);
        return;
    }

    lg_print_to_file(gbl_log, le);
    lg_print_to_queue(gbl_log, le);
    gbl_log->log_last = le;
}

