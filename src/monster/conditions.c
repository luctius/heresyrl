#include <assert.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/param.h>
#include <string.h>

#include "conditions.h"
#include "monster.h"


struct cdn_entry {
    struct cdn_condition *condition;
    LIST_ENTRY(cdn_entry) entries;
};

struct cdn_condition_list {
    LIST_HEAD(cdn_list_head, cdn_entry) head;
};

/* Garanteed to be random, rolled it myself ;)  */
#define CONDITION_PRE_CHECK (68731)
#define CONDITION_POST_CHECK (138)

struct cdn_condition_list *cdn_list_init(void) {
    struct cdn_condition_list *i = calloc(1, sizeof(struct cdn_condition_list) );
    if (i != NULL) {
        LIST_INIT(&i->head);
    }
    return i;
}

void cdn_list_exit(struct cdn_condition_list *cdn_list) {
    if (cdn_verify_condition(cdn_list) == false) return;

    struct cdn_entry *ce;
    while ( (ce = cdn_list->head.lh_first ) != NULL) {
        cdn_destroy(ce->condition);
        LIST_REMOVE(cdn_list->head.lh_first, entries);
        free(ce);
    }
    free(cdn_list);
}

void cdn_process(struct cdn_condition_list *cdn_list, struct msr_monster *monster) {
}

struct cdn_condition *cdn_create(uint32_t tid) {
}

void cdn_destroy(struct cdn_condition *cdn) {
    if (cdn_verify_condition(cdn) == false) return;
    free(cdn);
}

bool cdn_verify_condition(struct cdn_condition *cdn) {
    assert(cdn != NULL);
    assert(cdn->cdn_pre == CONDITION_PRE_CHECK);
    assert(cdn->cdn_post == CONDITION_POST_CHECK);

    return true;
}

struct cdn_condition *cdn_has_condition_type(struct cdn_condition_list *cdn_list, enum condition_type type) {
}

struct cdn_condition *cdn_has_condition_uid(struct cdn_condition_list *cdn_list, uint32_t uid) {
}

struct cdn_condition *cdn_has_condition_tid(struct cdn_condition_list *cdn_list, uint32_t tid) {
}

bool cdn_has_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *cdn) {
}

bool cdn_has_condition_type(struct cdn_condition_list *cdn_list, enum condition_type type) {
}

bool cdn_has_condition_uid(struct cdn_condition_list *cdn_list, uint32_t uid) {
}

bool cdn_has_condition_tid(struct cdn_condition_list *cdn_list, uint32_t tid) {
}

bool cdn_add_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *cdn) {
}

bool cdn_remove_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *cdn) {
}

/* Peek at items. if prev is NULL, it gives the first item, otherwise it gives the item after prev. */
struct cdn_condition *cdn_get_next_condition(struct cdn_condition_list *cdn_list, struct cdn_condition *prev) {
}

/* returns the number of total conditions in the condition list */
int cdn_condition_list_size(struct cdn_condition_list *cdn_list) {
}

