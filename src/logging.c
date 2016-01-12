/*
    This file is part of heresyRL.

    heresyRL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    heresyRL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with heresyRL.  If not, see <http://www.gnu.org/licenses/>.
*/

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

    /*struct queue *logging_q;*/
    /*int logging_q_sz;*/
    struct log_entry *log_q;
    struct cqc log_cqc;

    callback_event callback;
    void *priv;

    struct log_entry *log_last;
};

struct logging *lg_init(char *logfile, enum lg_debug_levels lvl, uint32_t max_size) {
    struct logging *log_ctx = calloc(1, sizeof(struct logging) );
    if (log_ctx == NULL) return NULL;

    log_ctx->level = lvl;
    cqc_init(log_ctx->log_cqc, max_size);
    log_ctx->log_q = malloc(cqc_qsz(log_ctx->log_cqc) * sizeof(struct log_entry) );

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

    while (cqc_cnt(log_ctx->log_cqc) > 0) {
        int idx = cqc_get(log_ctx->log_cqc);
        free(log_ctx->log_q[idx].module);
        free(log_ctx->log_q[idx].string);
    }
    free(log_ctx->log_q);

    fclose(log_ctx->log_file);
    free(log_ctx);
}

void lg_set_callback(struct logging *log_ctx, void *priv, callback_event ce) {
    if (log_ctx == NULL) return;

    log_ctx->priv = priv;
    log_ctx->callback = ce;
}

int lg_size(struct logging *log_ctx) {
    if (log_ctx == NULL) return -1;
    int sz = cqc_cnt(log_ctx->log_cqc);
    return sz;
}

void lg_add_entry(struct logging *log_ctx, struct log_entry *le_given) {
    if (cqc_space(log_ctx->log_cqc) == 0) {
        int idx = cqc_get(log_ctx->log_cqc);
        free(log_ctx->log_q[idx].module);
        free(log_ctx->log_q[idx].string);
    }

    int idx = cqc_put(log_ctx->log_cqc);

    struct log_entry *le = &log_ctx->log_q[idx];
    le->level  = le_given->level;
    le->repeat = le_given->repeat;
    le->module = le_given->module;
    le->string = le_given->string;
    le->line   = le_given->line;
    le->turn   = le_given->turn;
}

struct log_entry *lg_peek(struct logging *log_ctx, int idx) {
    if (idx >= lg_size(log_ctx) ) return NULL;
    int cqc_idx = cqc_peek(log_ctx->log_cqc, idx);
    struct log_entry *le = &log_ctx->log_q[cqc_idx];
    return le;
}

static bool le_is_equal(struct log_entry *a, struct log_entry *b) {
    if (a == NULL) return false;
    if (b == NULL) return false;

    if (a->level != b->level)   return false;
    if (strcmp(a->module, b->module) != 0) return false;

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
        case LG_DEBUG_LEVEL_GAME_INFO:
            pre_format = "Game Info";
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

    fprintf(fd, "[%s:%d][%s][%d] ", entry->module, entry->line, pre_format, entry->turn);
    fprintf(fd, "%s", entry->string);

    if (entry->repeat > 1) fprintf(fd, " x%d", entry->repeat);
    fprintf(fd, "\n");
    fflush(fd);
}

static void lg_print_to_queue(struct logging *log_ctx, struct log_entry *entry) {
    if (log_ctx == NULL) return;
    //if ( (options.debug == false) && (entry->level > LG_DEBUG_LEVEL_GAME_INFO) ) return;

    if ( (entry->level < LG_DEBUG_LEVEL_GAME_INFO) && (log_ctx->callback != NULL) ) log_ctx->callback(log_ctx, entry, log_ctx->priv);
}

#define STRING_MAX 500
void lg_printf_basic(struct logging *log_ctx, enum lg_debug_levels dbg_lvl, const char* module, int line, const char* format, va_list args) {
    if ( (log_ctx != NULL) && (dbg_lvl > log_ctx->level) ) return;

    if (cqc_space(log_ctx->log_cqc) == 0) {
        int idx = cqc_get(log_ctx->log_cqc);
        free(log_ctx->log_q[idx].module);
        free(log_ctx->log_q[idx].string);
    }

    int idx = cqc_put(log_ctx->log_cqc);

    struct log_entry *le = &log_ctx->log_q[idx];
    le->level  = dbg_lvl;
    le->repeat = 1;
    le->module = strdup(module);
    le->line   = line;
    le->turn   = 0;
    if (gbl_game != NULL) le->turn = gbl_game->turn;

    le->string = calloc(STRING_MAX, sizeof(char) );
    vsnprintf(le->string, STRING_MAX, format, args);
    le->string = realloc(le->string, strlen(le->string) +1);

    lg_print_to_file(log_ctx, le);
    lg_print_to_queue(log_ctx, le);
}

void lg_printf_l(int lvl, const char *module, int line, const char* format, ... ) {
    //if ( (gbl_log != NULL) && (lvl > gbl_log->level) ) return;

    va_list args;
    va_start(args, format);
    lg_printf_basic(gbl_log, lvl, module, line, format, args);
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

void msg_internal(coord_t *origin, coord_t *target, const char* module, int line, const char *format, ...) {
    if (gbl_log == NULL) return;


    /* check if the message is relevant. */
    if (msg_valid(origin, target) == false) return;

    /* Message is accepted, either because it is a system message,
       or because either the origin or target are of interrest to the
       player.
    */

    /* copy the string from the vararg list */
    char buf[STRING_MAX];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, STRING_MAX, format, args);
    va_end(args);

    struct log_entry log_entry;
    struct log_entry *le = &log_entry;
    le->repeat  = 1;
    le->module  = strdup(module);
    le->line    = line;
    le->level   = LG_DEBUG_LEVEL_GAME;
    le->turn    = gbl_game->turn;

    le->string = buf; /* temp copy */
    if (le_is_equal(le, gbl_log->log_last) ) {
        gbl_log->log_last->repeat++;
        gbl_log->log_last->turn = gbl_game->turn;

        /* There is no new entry, so we have to force an update ourselves */
        if (gbl_log->callback != NULL) gbl_log->callback(gbl_log, gbl_log->log_last, gbl_log->priv);
        return;
    }

    /* We really need a real copy now. */
    le->string = strdup(buf);

    if (cqc_space(gbl_log->log_cqc) == 0) {
        int idx = cqc_get(gbl_log->log_cqc);
        free(gbl_log->log_q[idx].module);
        free(gbl_log->log_q[idx].string);
    }

    int idx = cqc_put(gbl_log->log_cqc);
    le = &gbl_log->log_q[idx];
    memcpy(le, &log_entry, sizeof(struct log_entry) );

    lg_print_to_file(gbl_log, le);
    lg_print_to_queue(gbl_log, le);
    gbl_log->log_last = le;
}

int clrstr_len(const char *txt) {
    if (strncmp(CS_COLOUR_PRE, txt, strlen(CS_COLOUR_PRE) ) != 0) return -1;

    if (strncmp(cs_CLOSE,         txt, strlen(cs_CLOSE) )    == 0) return strlen(cs_CLOSE);
    else if (strncmp(cs_MONSTER,  txt, strlen(cs_MONSTER) )  == 0) return strlen(cs_MONSTER);
    else if (strncmp(cs_PLAYER,   txt, strlen(cs_PLAYER) )   == 0) return strlen(cs_PLAYER);
    else if (strncmp(cs_ITEM  ,   txt, strlen(cs_ITEM) )     == 0) return strlen(cs_ITEM);
    else if (strncmp(cs_DAMAGE,   txt, strlen(cs_DAMAGE) )   == 0) return strlen(cs_DAMAGE);
    else if (strncmp(cs_WARNING,  txt, strlen(cs_WARNING) )  == 0) return strlen(cs_WARNING);
    else if (strncmp(cs_CRITICAL, txt, strlen(cs_CRITICAL) ) == 0) return strlen(cs_CRITICAL);
    else if (strncmp(cs_GM,       txt, strlen(cs_GM) )       == 0) return strlen(cs_GM);
    else if (strncmp(cs_SYSTEM,   txt, strlen(cs_SYSTEM) )   == 0) return strlen(cs_SYSTEM);
    else if (strncmp(cs_ATTR,     txt, strlen(cs_ATTR) )     == 0) return strlen(cs_ATTR);
    else if (strncmp(cs_OLD,      txt, strlen(cs_OLD) )      == 0) return strlen(cs_OLD);
    return -1;
}

bool clrstr_is_colour(const char *txt) {
    if (strncmp(CS_COLOUR_PRE, txt, strlen(CS_COLOUR_PRE) ) != 0) return false;
    return (clrstr_to_attr(txt) != get_colour(TERM_COLOUR_L_WHITE) );
}

bool clrstr_is_close(const char *txt) {
    if (strncmp(CS_COLOUR_PRE, txt, strlen(CS_COLOUR_PRE) ) != 0) return false;
    if (strncmp(cs_CLOSE, txt, strlen(cs_CLOSE) )  == 0) return true;
    return false;
}

int clrstr_to_attr(const char *s) {
    if (strncmp(cs_MONSTER,       s, strlen(cs_MONSTER) )  == 0) return get_colour(TERM_COLOUR_L_BLUE);
    else if (strncmp(cs_PLAYER,   s, strlen(cs_PLAYER) )   == 0) return get_colour(TERM_COLOUR_L_GREEN);
    else if (strncmp(cs_ITEM  ,   s, strlen(cs_ITEM) )     == 0) return get_colour(TERM_COLOUR_VIOLET);
    else if (strncmp(cs_DAMAGE,   s, strlen(cs_DAMAGE) )   == 0) return get_colour(TERM_COLOUR_L_VIOLET);
    else if (strncmp(cs_WARNING,  s, strlen(cs_WARNING) )  == 0) return get_colour(TERM_COLOUR_L_YELLOW);
    else if (strncmp(cs_CRITICAL, s, strlen(cs_CRITICAL) ) == 0) return get_colour(TERM_COLOUR_RED);
    else if (strncmp(cs_GM,       s, strlen(cs_GM) )       == 0) return get_colour(TERM_COLOUR_L_PURPLE);
    else if (strncmp(cs_SYSTEM,   s, strlen(cs_SYSTEM) )   == 0) return get_colour(TERM_COLOUR_WHITE);
    else if (strncmp(cs_ATTR,     s, strlen(cs_ATTR) )     == 0) return get_colour(TERM_COLOUR_L_TEAL);
    else if (strncmp(cs_OLD,      s, strlen(cs_OLD) )      == 0) return get_colour(TERM_COLOUR_L_DARK);

    return get_colour(TERM_COLOUR_L_WHITE);
}

