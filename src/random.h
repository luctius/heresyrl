#ifndef RANDOM_H_
#define RANDOM_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

struct random;

struct random *random_init_genrand(unsigned long s);
void random_exit(struct random *r);
unsigned long random_genrand_int32(struct random *r);

int random_get_nr_called(struct random *r);
void random_loop_called(struct random *r, int called);

#endif /*RANDOM_H_*/
