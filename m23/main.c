#include "./graph.h"
#include "algos.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main()
{
    srand(time(NULL));

    graphe g;
    init_graphe(&g);

    printf("AFFICHER GRAPHE\n");
    printf("===============\n");
    afficher(&g);

    free_graphe(&g);

    return EXIT_SUCCESS;
}
