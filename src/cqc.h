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

#define cqc_init(cqc, sz) do { (cqc).size = sz; (cqc).tail = 0; (cqc).head = 0; } while (0)
#define cqc_cnt(cqc)        ( ((cqc).head-(cqc).tail) % ((cqc).size-1) )
#define cqc_put(cqc)        ( ((cqc).head++)          % ((cqc).size-1) )
#define cqc_get(cqc)        ( ((cqc).tail++)          % ((cqc).size-1) )
#define cqc_peek(cqc, idx)  ( ((cqc).tail+idx)        % ((cqc).size-1) )
#define cqc_space(cqc)      ( ((cqc).size-1)          - (cqc_cnt(cqc)) )

#elif !defined(CQC_USE_MODULO)

/* round to the power of two */
#define cqc_rnd_to_pwr2(sz) do { sz--; sz |= sz>>1; sz |= sz>>2; sz |= sz>>4; sz |= sz>>8; sz |= sz>>16; sz++; } while (0)

#define cqc_init(cqc, sz) do { cqc_rnd_to_pwr2(sz); (cqc).size = sz; (cqc).tail = 0; (cqc).head = 0; } while (0)
#define cqc_cnt(cqc)        ( ((cqc).head-(cqc).tail) & ((cqc).size-1) )
#define cqc_put(cqc)        ( ((cqc).head++)          & ((cqc).size-1) )
#define cqc_get(cqc)        ( ((cqc).tail++)          & ((cqc).size-1) )
#define cqc_peek(cqc, idx)  ( ((cqc).tail+idx)        & ((cqc).size-1) )
#define cqc_space(cqc)      ( ((cqc).size-1)          - (cqc_cnt(cqc)) )

#endif /* CQC_USE_MODULO */

#endif /*CQC_H*/
