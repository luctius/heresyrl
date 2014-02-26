#pragma once
#ifndef RANDOM_H_
#define RANDOM_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

struct random;

struct random *random_init_genrand(unsigned long s);
struct random *random_init_by_array(unsigned long init_key[], int key_length);
void random_exit(struct random *r);
unsigned long random_genrand_int32(struct random *r);
long random_genrand_int31(struct random *r);
double random_genrand_real1(struct random *r);
double random_genrand_real2(struct random *r);
double random_genrand_real3(struct random *r);
double random_genrand_res53(struct random *r);

#endif /*RANDOM_H_*/
