#pragma once
#ifndef CQC_H
#define CQC_H

struct cqc {
    int head;
    int tail;
    int size;
};

#define cqc_init(cqc, sz) do { (cqc).size = sz; (cqc).tail = 0; (cqc).head = 0; } while (0)
#define cqc_cnt(cqc) ( (cqc).head - (cqc).tail)
#define cqc_space(cqc) ( ( (cqc).size - cqc_cnt(cqc) ) -1)
#define cqc_put(cqc) ( (cqc).head++ ) % ( (cqc).size -1)
#define cqc_get(cqc) ( (cqc).tail++ ) % ( (cqc).size -1)
#define cqc_peek(cqc, idx) ( (cqc).tail + idx ) % ( (cqc).size -1)

#endif /*CQC_H*/
