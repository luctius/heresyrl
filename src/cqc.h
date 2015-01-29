#pragma once
#ifndef CQC_H
#define CQC_H

struct cqc {
    int head;
    int tail;
    int size;
};

//#define CQC_USE_MODULO /* Define this if you need arbitrary size queues instead of power of two sizes. */
#ifdef CQC_USE_MODULO

    /* Init Circular buffer. In the case of Module, given size is the size of the buffer. */
#define cqc_init(cqc, sz) do { (cqc).size = sz; (cqc).tail = 0; (cqc).head = 0; } while (0)
    /* Size given with init. */
#define cqc_qsz(cqc)        ( (cqc).size )
    /* Number of Items in the buffer. */
#define cqc_cnt(cqc)        ( ((cqc).head-(cqc).tail) % ((cqc).size) )
    /* Add new item, returns a free index and updates bookkeeping. */
#define cqc_put(cqc)        ( ((cqc).head++)          % ((cqc).size) )
    /* Retreive item, returns an used index and updates bookkeeping. */
#define cqc_get(cqc)        ( ((cqc).tail++)          % ((cqc).size) )
    /* Retreive item idx, does not touch bookkeeping. */
#define cqc_peek(cqc, idx)  ( ((cqc).tail+idx)        % ((cqc).size) )
    /* Space left in the buffer. */
#define cqc_space(cqc)      ( ((cqc).size)            - (cqc_cnt(cqc)) )

#define cqc_cnt_to_end(cqc)     ({int end = (cqc.size) - (cqc.tail); int n = ((cqc.head) + end) & ((cqc.size)-1); n < end ? n : end;})
#define cqc_space_to_end(cqc)   ({int end = (cqc.size) - 1 - (cqc.head); int n = (end + (cqc.tail)) & ((cqc.size)-1); n <= end ? n : end+1;})

#elif !defined(CQC_USE_MODULO)

/* round to the power of two */
#define cqc_rnd_to_pwr2(sz) do { if ( (sz & (sz-1) ) != 0) { sz |= sz>>1; sz |= sz>>2; sz |= sz>>4; sz |= sz>>8; sz |= sz>>16; sz++; } } while (0)

    /* Init Circular buffer. In the case of AND, given size is the nearest power of 2 of the given size. cqc_qsz should be used to initialise the buffer. */
#define cqc_init(cqc, sz) do { uint32_t sz2 = (sz); cqc_rnd_to_pwr2(sz2); (cqc).size = sz2-1; (cqc).tail = 0; (cqc).head = 0; } while (0)
    /* Size calculated in init. */
#define cqc_qsz(cqc)        ( (cqc).size+1 )
    /* Number of Items in the buffer. */
#define cqc_cnt(cqc)        ( ((cqc).head-(cqc).tail) & ((cqc).size) )
    /* Retreive item, returns an used index and updates bookkeeping. */
#define cqc_put(cqc)        ( ((cqc).head++)          & ((cqc).size) )
    /* Retreive item, returns an used index and updates bookkeeping. */
#define cqc_get(cqc)        ( ((cqc).tail++)          & ((cqc).size) )
    /* Retreive item idx, does not touch bookkeeping. */
#define cqc_peek(cqc, idx)  ( ((cqc).tail+idx)        & ((cqc).size) )
    /* Space left in the buffer. */
#define cqc_space(cqc)      ( ((cqc).size)            - (cqc_cnt(cqc)) )

#define cqc_cnt_to_end(cqc)     ({int end = (cqc.size) - (cqc.tail); int n = ((cqc.head) + end) & ((cqc.size)-1); n < end ? n : end;})
#define cqc_space_to_end(cqc)   ({int end = (cqc.size) - 1 - (cqc.head); int n = (end + (cqc.tail)) & ((cqc.size)-1); n <= end ? n : end+1;})

#endif /* CQC_USE_MODULO */

#endif /*CQC_H*/
