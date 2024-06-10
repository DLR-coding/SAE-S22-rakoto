#include "utils.h"

void generer_permutation(size_t *permutation, size_t n)
{
  for (size_t i = 0; i < n; ++i) {
        permutation[i] = i;
    }

    // Mélanger le tableau pour obtenir une permutation aléatoire
    for (size_t i = n - 1; i > 0; --i) {
        size_t j = rand() % (i + 1);
        size_t temp = permutation[i];
        permutation[i] = permutation[j];
        permutation[j] = temp;
    }
}

void init_aleatoire(double *val, size_t n, double max)
{
    for (size_t i = 0; i < n; ++i) {
        val[i] = ((double) rand() / RAND_MAX) * max;
    }
}
