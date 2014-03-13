#ifndef RANDOM_H_
#define RANDOM_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

struct random;

struct random *random_init_genrand(unsigned long s);
void random_exit(struct random *r);

int random_xd5(struct random *r, int nr_die);
int random_xd10(struct random *r, int nr_die);
int random_d100(struct random *r);

unsigned long random_int32(struct random *r);
double random_float(struct random *r);

int random_get_seed(struct random *r);
int random_get_nr_called(struct random *r);
void random_loop_called(struct random *r, int called);
char *random_die_name(int die_sz);

#endif /*RANDOM_H_*/
