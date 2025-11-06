#include "../include/random.h"
#include "../include/time.h"


void srand()
{
    seed = current_seconds();
}

double rand()
{
    return (seed = (seed * 1103515245 + 12345) & RAND_MAX) * NORM;
}