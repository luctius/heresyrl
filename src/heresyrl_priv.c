#include <stdlib.h>
#include <stdint.h>
#include <math.h>

int pyth(int side1, int side2) {
    //return sqrt(pow(side1, 2) + pow(side2, 2) );
    int a = abs(side1);
    int b = abs(side2);
    return (a > b) ? a : b;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

