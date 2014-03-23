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
#include "monster.h"
#include "dungeon_creator.h"

struct logging *gbl_log = NULL;

struct logging {
    FILE *log_file;

    enum lg_debug_levels level;
    struct queue *logging_q;

    int logging_q_sz;

    callback_event callback;
    void *priv;

    bool log_fds_active[MSG_MAX_FD];
    struct log_entry *log_fd;
    struct log_entry *log_last;
};

struct logging *lg_init(char *logfile, enum lg_debug_levels lvl, int max_size) {
    struct logging *log = calloc(1, sizeof(struct logging) );
    if (log == NULL) return NULL;

    log->level = lvl;
    log->logging_q_sz = max_size;
    log->logging_q = queue_init_simple(log->logging_q_sz+1);
    log->log_file = fopen(logfile, "w");
    if (log->log_file == NULL) {
        fprintf(stderr, "Could not open logfile %s\n", logfile);
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

struct queue *lg_queue(struct logging *log) {
    return log->logging_q;
}

static bool le_is_equal(struct log_entry *a, struct log_entry *b) {
    if (a == NULL) return false;
    if (b == NULL) return false;
    if (a->atom_lst == NULL) return false;
    if (b->atom_lst == NULL) return false;

    if (a->level != b->level)   return false;
    if (a->module != b->module) return false;
    if (a->atom_lst_sz != b->atom_lst_sz) return false;

    for (int i = 0; i < a->atom_lst_sz; i++) {
        struct log_atom *aa = &a->atom_lst[i];
        struct log_atom *ab = &b->atom_lst[i];
        if (aa->string == NULL) return false;
        if (ab->string == NULL) return false;

        if (aa->channel != ab->channel) return false;
        if (strcmp(aa->string, ab->string) != 0) return false;
    }

    return true;
}

static void le_free(struct log_entry *e) {
    for (int i = 0; i < e->atom_lst_sz; i++) {
        free(e->atom_lst[i].string);
    }
    free(e->atom_lst);
    free(e);
}

static void lg_print_to_file(struct logging *log, struct log_entry *entry) {
    FILE *fd = stderr;
    char *pre_format = "";

    if (log != NULL)
    {
        if (entry->level > log->level) return;
        fd = log->log_file;
    }

    if (fd == NULL && (entry->level == LG_DEBUG_LEVEL_ERROR || entry->level == LG_DEBUG_LEVEL_WARNING || entry->level == LG_DEBUG_LEVEL_GAME) ) fd = stderr;
    else if (fd == NULL) return;

    if (entry->level >= LG_DEBUG_LEVEL_MAX) entry->level = LG_DEBUG_LEVEL_DEBUG;

    switch (entry->level)
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

    fprintf(fd, pre_format, entry->module);
    for (int i = 0; i < entry->atom_lst_sz; i++) {
        fprintf(fd, "%s", entry->atom_lst[i].string);
    }
    if (entry->repeat > 0) fprintf(fd, " x%d", entry->repeat);
    fprintf(fd, "\n");
    fflush(fd);
}

static void lg_print_to_queue(struct logging *log, struct log_entry *entry) {
    if (log == NULL) return;
    //if (dbg_lvl > LG_DEBUG_LEVEL_GAME) return;

    queue_push_tail(log->logging_q, (intptr_t) entry);

    if (log->callback != NULL) log->callback(log, entry, log->priv);

    while (queue_size(log->logging_q) > log->logging_q_sz) {
        struct log_entry *tmp = (struct log_entry *) queue_pop_head(log->logging_q);
        le_free(tmp);
    }
}

#define STRING_MAX 500
void lg_printf_basic(struct logging *log, enum lg_debug_levels dbg_lvl, const char* module, const char* format, va_list args) {

    if ( (log != NULL) && (dbg_lvl > log->level) ) return;
    struct log_entry *le = calloc(1, sizeof(struct log_entry) );
    if (le == NULL) return;

    le->level = dbg_lvl;
    le->repeat = 0;
    le->module = module;
    le->turn = 0;
    if (gbl_game != NULL) le->turn = gbl_game->turn;

    le->atom_lst_sz = 1;
    le->atom_lst = calloc(le->atom_lst_sz, sizeof(struct log_atom) );
    if (le->atom_lst == NULL) {
        free (le->atom_lst);
        return;
    }

    struct log_atom *la = &le->atom_lst[0];
    la->channel = LG_CHANNEL_DEBUG;
    if ( (dbg_lvl == LG_DEBUG_LEVEL_ERROR) || (dbg_lvl == LG_DEBUG_LEVEL_WARNING) ) {
        la->channel = LG_CHANNEL_WARNING;
    }
    
    la->string = calloc(STRING_MAX, sizeof(char) );
    vsnprintf(la->string, STRING_MAX, format, args);
    la->string = realloc(la->string, strlen(la->string) +1);

    lg_print_to_file(log, le);
    lg_print_to_queue(log, le);
}

void lg_printf_l(int lvl, const char *module, const char* format, ... ) {
    va_list args;
    va_start(args, format);
    lg_printf_basic(gbl_log, lvl, module , format, args);
    va_end(args);
}

static void msg_init_internal(void) {
    assert(gbl_log != NULL);
    assert(gbl_game != NULL);

    free(gbl_log->log_fd);
    for (int i = 0; i < MSG_MAX_FD; i++) {
        gbl_log->log_fds_active[i] = false;
    }

    gbl_log->log_fd = calloc(1, sizeof(struct log_entry) );
    struct log_entry *le = gbl_log->log_fd;

    le->turn = gbl_game->turn;
    le->repeat = 1;
    le->module = "game";
    le->atom_lst_sz = 0;
    le->level = LG_DEBUG_LEVEL_GAME;
    le->atom_lst = NULL;
}

void msg_init(struct msr_monster *m, struct msr_monster *t) {
    msg_init_internal();

    /* check if the message should be accepted; 
       which is if the player is eiter involved, 
       or if he can see it. But m should NOT be 
       player itself, otherwise we have 2x the
       messages */
    if (m->is_player == true) {
        gbl_log->log_fds_active[MSG_PLR_FD] = true;
        return;
    }

    struct dc_map_entity *me = sd_get_map_me(&m->pos, gbl_game->current_map);
    if (me->visible) gbl_log->log_fds_active[MSG_MSR_FD] = true;
    else if (t != NULL) {
        me = sd_get_map_me(&t->pos, gbl_game->current_map);
        if (me->visible) gbl_log->log_fds_active[MSG_MSR_FD] = true;
    }
}


void msg_exit(void) {
    bool active = false;
    for (int i = 0; i < MSG_MAX_FD; i++) {
        if (gbl_log->log_fds_active[i]) {
            active = true;
        }

        gbl_log->log_fds_active[i] = false;
    }
    if (active) {
        struct log_entry *le = gbl_log->log_fd;
        if (le_is_equal(le, gbl_log->log_last) ) {
            gbl_log->log_last->repeat++;
            gbl_log->log_last->turn = gbl_game->turn;
            le_free(le);
            gbl_log->log_fd = NULL;

            /* There is no new entry, so we have to force an update ourselves */
            if (gbl_log->callback != NULL) gbl_log->callback(gbl_log, le, gbl_log->priv);
        }
        else if (le->atom_lst_sz > 0) {
            lg_print_to_file(gbl_log, le);
            lg_print_to_queue(gbl_log, le);
            gbl_log->log_last = gbl_log->log_fd;
            gbl_log->log_fd = NULL;
        }
    }
}

void msg_add(enum msg_fd fd, enum lg_channel c, const char *format, ...) {
    assert(gbl_log != NULL);
    if (gbl_log->log_fds_active[fd] == false) return;
    struct log_entry *le = gbl_log->log_fd;

    le->atom_lst_sz++;
    le->atom_lst = realloc(le->atom_lst, le->atom_lst_sz * sizeof(struct log_atom) );
    if (le->atom_lst == NULL) {
        le->atom_lst_sz -= 1;
        return;
    }

    le->atom_lst[le->atom_lst_sz -1].channel = c;
    le->turn = gbl_game->turn;

    struct log_atom *la = &le->atom_lst[le->atom_lst_sz -1];
    la->string = calloc(STRING_MAX, sizeof(char) );
    if (la->string == NULL) {
        le->atom_lst_sz -= 1;
        return;
    }

    va_list args;
    va_start(args, format);
    vsnprintf(la->string, STRING_MAX, format, args);
    va_end(args);

    la->string = realloc(la->string, strlen(la->string) +1);
}

