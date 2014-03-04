#ifndef RANDOM_H_
#define RANDOM_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

struct random;

struct random *random_init_genrand(unsigned long s);
void random_exit(struct random *r);
unsigned long random_genrand_int32(struct random *r);

#endif /*RANDOM_H_*/
