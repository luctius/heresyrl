#pragma once
#ifndef QUEUE_H
#define QUEUE_H

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#define QUEUE_SUCCESS (0)

/*
   TODO replace current queue with this
struct cqc {
    int head;
    int tail;
    int size;
};

#define cqc_init(cqc, size) do { (cqc).size = size; (cqc).tail = 0; (cqc).head = 0; } while (0)
#define cqc_cnt(cqc) ( (cqc).head - (cqc).tail) & ( (cqc).size -1)
#define cqc_space(cqc) ( ( (cqc).tail) - (cqc).head+1) & ( (cqc).size -1)
#define cqc_put(cqc) ( (cqc).head++ ) & ( (cqc).size -1)
#define cqc_get(cqc) ( (cqc).tail++ ) & ( (cqc).size -1)
*/

union qe {
    uint64_t ui;
    int64_t i;
    void *vp;
    double d;
};

struct queue;
int queue_minimum_size(void);
int queue_element_size(void);

struct queue *queue_init_simple(int nr_elements) __attribute__ ((warn_unused_result));
struct queue *queue_init(void *mem, size_t sz) __attribute__ ((warn_unused_result));

int queue_exit(struct queue *q);
int queue_push_tail(struct queue *q, union qe element);
union qe queue_pop_head(struct queue *q) __attribute__ ((warn_unused_result));
union qe queue_peek_head(struct queue *q) __attribute__ ((warn_unused_result));
union qe queue_peek_nr(struct queue *q, int nr) __attribute__ ((warn_unused_result));
union qe queue_peek_tail(struct queue *q) __attribute__ ((warn_unused_result));

int queue_size(struct queue *q);
int queue_free(struct queue *q);

#endif /*QUEUE_H*/
