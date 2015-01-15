#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "queue.h"

#include "lro2_os.h"

//#define Q_DEBUG 1

#define QUEUE_BAIL_PRINTF(fmt, ...) {fprintf(stderr, "\033[0;31mQUEUE-> Error: [%s:%s] " fmt "\033[0m\n", __FILE__, __func__,  ##__VA_ARGS__);}
#define QUEUE_BAIL_NULL(fmt, ...) { QUEUE_BAIL_PRINTF(fmt, ##__VA_ARGS__); return NULL; }
#define QUEUE_BAIL_INT(error, fmt, ...) { QUEUE_BAIL_PRINTF(fmt, ##__VA_ARGS__); return error; }
#define QUEUE_BAIL(error, fmt, ...) { QUEUE_BAIL_PRINTF(fmt, ##__VA_ARGS__); union qe err = { .i = error, }; return err; }

struct queue {
    void *mem_ptr;
    size_t mem_sz;

    lro2_sem_t semaphore;

    int nr_elements;
    int element_ctr;;
    int head;
    int tail;
    bool owner;
    union qe queue_start[];
};

int queue_free_unsafe(struct queue *q);
int queue_size_unsafe(struct queue *q);

inline int queue_element_size(void) {
    return (sizeof(union qe) );
}

inline int queue_minimum_size(void) {
    return sizeof(struct queue) +(2 * queue_element_size() );
}

static struct queue *queue_init_priv(void *mem, size_t sz, bool owner) {
    struct queue *q = mem;

    if (mem == NULL) QUEUE_BAIL_NULL("Parameter 'mem' invalid.");
    if (sz < sizeof(struct queue) + (2 * sizeof(union qe) ) ) QUEUE_BAIL_NULL("Parameter 'sz' invalid.");
    if (lro2_sem_init(&q->semaphore, 0, 1) != 0) QUEUE_BAIL_NULL("Requesting semaphore failed.");

    q->mem_ptr = mem;
    q->mem_sz = sz;

    q->nr_elements = ( ( sz - sizeof(struct queue) ) / sizeof(union qe) ) -2;
    q->head = 0;
    q->tail = 0;
    q->element_ctr = 0;
    q->owner = owner;

    return q;
}

struct queue *queue_init_simple(int nr_elements) {
    size_t sz = (nr_elements * queue_element_size() ) + queue_minimum_size();
    void *mem = malloc(sz);
    return queue_init_priv(mem, sz, true);
}

struct queue *queue_init(void *mem, size_t sz) {
    return queue_init_priv(mem, sz, false);
}

int queue_exit(struct queue *q) {
    if (q == NULL) QUEUE_BAIL_INT(-EINVAL, "Queue context invalid.");
    if (q->owner == true) {
        free(q);
    }

    return QUEUE_SUCCESS;
}

int queue_push_tail(struct queue *q, union qe element) {
    int retval = -ENODATA;
    if (q == NULL) QUEUE_BAIL_INT(-EINVAL, "Queue context invalid.");
    lro2_sem_wait(&q->semaphore); /*sem down*/

    if (queue_free_unsafe(q) > 0) {
        q->queue_start[q->tail] = element;
        q->tail = (q->tail +1 ) % q->nr_elements;
        q->element_ctr++;

#ifdef Q_DEBUG
        fprintf(stderr,"push [%p] [val: 0x%x] [%d/%d->%d:%d]\n", q, element, q->element_ctr, q->nr_elements, q->head, q->tail);
#endif
        retval = QUEUE_SUCCESS;
    }

    lro2_sem_post(&q->semaphore); /*sem up*/
    return retval;
}

union qe queue_pop_head(struct queue *q) {
    union qe retval = { .i= -ENODATA };
    if (q == NULL) QUEUE_BAIL(-EINVAL, "Queue context invalid.");
    lro2_sem_wait(&q->semaphore); /*sem down*/

    if (queue_size_unsafe(q) > 0) {
        retval = q->queue_start[q->head];
        union qe e = { .i = 0xDEADBEEF, };
        q->queue_start[q->head] = e;
        q->head = ( q->head +1 ) % q->nr_elements;
        q->element_ctr--;

        if (queue_size_unsafe(q) == 0) {
            q->head = 0;
            q->tail = 0;
        }
#ifdef Q_DEBUG
        fprintf(stderr, "pop [%p] [val: 0x%X] [%d/%d->%d:%d]\n", q, retval, q->element_ctr, q->nr_elements, q->head, q->tail);
#endif
    }

    lro2_sem_post(&q->semaphore); /*sem up*/
    return retval;
}

union qe queue_peek_head(struct queue *q) {
    return queue_peek_nr(q, 0);
}

union qe queue_peek_nr(struct queue *q, int nr) {
    union qe retval = { .i = -ENODATA};
    if (q == NULL) QUEUE_BAIL(-EINVAL, "Queue context invalid.");
    if (nr < 0) QUEUE_BAIL(-EINVAL, "Invalid parameter.");
    lro2_sem_wait(&q->semaphore); /*sem down*/

    if (queue_size_unsafe(q) > 0 && nr < queue_size_unsafe(q) ) {
        int pos = (q->head + nr) % q->nr_elements;
        retval = q->queue_start[pos];

#ifdef Q_DEBUG
        fprintf(stderr, "peek [%p] [val: 0x%X (%d -> %d)] [%d/%d->%d:%d]\n", q, retval, nr, pos, q->element_ctr, q->nr_elements, q->head, q->tail);
#endif
    }

    lro2_sem_post(&q->semaphore); /*sem up*/
    return retval;
}

union qe queue_peek_tail(struct queue *q) {
    int pt = (q->nr_elements + q->tail -1) % q->nr_elements;
    return queue_peek_nr(q, pt);
}

int queue_size_unsafe(struct queue *q) {
    int nre = 0;
    if (q == NULL) QUEUE_BAIL_INT(-EINVAL, "Queue context invalid.");
    nre = q->element_ctr;
    return nre;
}

int queue_size(struct queue *q) {
    int nre = 0;
    lro2_sem_wait(&q->semaphore); /*sem down*/
    nre = queue_size_unsafe(q);
    lro2_sem_post(&q->semaphore); /*sem up*/
    return nre;
}

int queue_free_unsafe(struct queue *q) {
    int nre = 0;
    if (q == NULL) QUEUE_BAIL_INT(-EINVAL, "Queue context invalid.");
    nre = q->nr_elements - q->element_ctr;
    return nre;
}

int queue_free(struct queue *q) {
    int nre = 0;
    lro2_sem_wait(&q->semaphore); /*sem down*/
    nre = queue_free_unsafe(q);
    lro2_sem_post(&q->semaphore); /*sem up*/
    return nre;
}

