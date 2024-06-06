#include "utils.h"

void generer_permutation(size_t *permutation, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        int val = rand()% n-1;
        permutation[i] = val;
    }

}

void init_aleatoire(double *val, size_t n, double max)
{
}
