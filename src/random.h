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

/* TODO: implement http://stackoverflow.com/questions/288739/generate-random-numbers-uniformly-over-an-entire-range/288869#288869 */
unsigned long random_int32(struct random *r);
double random_float(struct random *r);

unsigned long random_get_seed(struct random *r);
int random_get_nr_called(struct random *r);
void random_loop_called(struct random *r, int called);
const char *random_die_name(int die_sz);

#endif /*RANDOM_H_*/
