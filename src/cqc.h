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

#pragma once
#ifndef CQC_H
#define CQC_H

#include <stdint.h>

typedef int64_t cqcunit_t;

struct cqc {
    cqcunit_t head;
    cqcunit_t tail;
    cqcunit_t size;
};

/* round to the power of two */
#define cqc_rnd_to_pwr2(sz) do { if ( (sz & (sz-1) ) != 0) { sz |= sz>>1; sz |= sz>>2; sz |= sz>>4; sz |= sz>>8; sz |= sz>>16; sz++; } } while (0)

//#define CQC_USE_MODULO /* Define this if you need arbitrary size queues instead of power of two sizes. */
#ifdef CQC_USE_MODULO

    /* Init Circular buffer. In the case of Module, given size is the size of the buffer. */
    #define cqc_init(cqc, sz) do { (cqc).size = sz-1; (cqc).tail = 0; (cqc).head = 0; } while (0)
    /* Size given with init. */
    #define cqc_qsz(cqc)        ( (cqc).size )

#elif !defined(CQC_USE_MODULO)

    /* Init Circular buffer. In the case of AND, given size is the nearest power of 2 of the given size. cqc_qsz should be used to initialise the buffer. */
    #define cqc_init(cqc, sz) do { cqcunit_t sz2 = (sz); cqc_rnd_to_pwr2(sz2); (cqc).size = sz2-1; (cqc).tail = 0; (cqc).head = 0; } while (0)
    /* Size calculated in init. */
#endif /* CQC_USE_MODULO */

#define cqc_qsz(cqc)        ( (cqc).size+1 )

/* Number of Items in the buffer. */
#define cqc_cnt(cqc)            ( ((cqc).head-(cqc).tail) & ((cqc).size) )
/* Retreive item, returns an used index and updates bookkeeping. */
#define cqc_put(cqc)            ( (cqc).head = ( ( (cqc).head + 1) & (cqc).size ) )
#define cqc_put_cnt(cqc,cnt)    ( (cqc).head = ( ( (cqc).head + cnt) & (cqc).size ) )
/* Retreive item, returns an used index and updates bookkeeping. */
#define cqc_get(cqc)            ( (cqc).tail = ( ( (cqc).tail + 1) & (cqc).size ) )
#define cqc_get_cnt(cqc, cnt)   ( (cqc).tail = ( ( (cqc).tail + cnt) & (cqc).size ) )
/* Retreive item idx, does not touch bookkeeping. */
#define cqc_peek(cqc, idx)  ( ((cqc).tail+1+idx)        & ((cqc).size) )
/* Space left in the buffer. */
#define cqc_space(cqc)      ( ((cqc).size)            - (cqc_cnt(cqc)) )

#define cqc_cnt_to_end(cqc)     ({cqcunit_t cqcend = (cqc.size) - (cqc.tail); cqcunit_t cqcn = ((cqc.head) + cqcend) & ((cqc.size)-1); cqcn < cqcend ? cqcn : cqcend;})
#define cqc_space_to_end(cqc)   ({cqcunit_t cqcend = (cqc.size) - 1 - (cqc.head); cqcunit_t cqcn = (cqcend + (cqc.tail)) & ((cqc.size)-1); cqcn <= cqcend ? cqcn : cqcend+1;})


#endif /*CQC_H*/
