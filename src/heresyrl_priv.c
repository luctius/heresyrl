#include <stdlib.h>
#include <stdint.h>
#include <math.h>

int pyth(int side1, int side2) {
    //return sqrt(pow(side1, 2) + pow(side2, 2) );
    return (abs(side1) > abs(side2)) ? abs(side1) : abs(side2);
}
