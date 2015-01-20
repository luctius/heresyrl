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
#define cqc_qsz(cqc)        ( (cqc).size )
#define cqc_cnt(cqc)        ( ((cqc).head-(cqc).tail) % ((cqc).size) )
#define cqc_put(cqc)        ( ((cqc).head++)          % ((cqc).size) )
#define cqc_get(cqc)        ( ((cqc).tail++)          % ((cqc).size) )
#define cqc_peek(cqc, idx)  ( ((cqc).tail+idx)        % ((cqc).size) )
#define cqc_space(cqc)      ( ((cqc).size)            - (cqc_cnt(cqc)) )

#elif !defined(CQC_USE_MODULO)

/* round to the power of two */
#define cqc_rnd_to_pwr2(sz) do { sz |= sz>>1; sz |= sz>>2; sz |= sz>>4; sz |= sz>>8; sz |= sz>>16; sz++; } while (0)

#define cqc_init(cqc, sz) do { uint32_t sz2 = (sz); if ( (sz2 & (sz2-1) ) != 0) cqc_rnd_to_pwr2(sz2); (cqc).size = sz2-1; (cqc).tail = 0; (cqc).head = 0; } while (0)
#define cqc_qsz(cqc)        ( (cqc).size+1 )
#define cqc_cnt(cqc)        ( ((cqc).head-(cqc).tail) & ((cqc).size) )
#define cqc_put(cqc)        ( ((cqc).head++)          & ((cqc).size) )
#define cqc_get(cqc)        ( ((cqc).tail++)          & ((cqc).size) )
#define cqc_peek(cqc, idx)  ( ((cqc).tail+idx)        & ((cqc).size) )
#define cqc_space(cqc)      ( ((cqc).size)            - (cqc_cnt(cqc)) )

#endif /* CQC_USE_MODULO */

#endif /*CQC_H*/
