#include "./graphe.h"
#include "algos.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


int main() {
    srand(time(NULL));

    graphe g;
    init_graphe(&g);

    printf("AFFICHER GRAPHE\n");
    printf("===============\n");
    ajouter_sommet(&g);
    ajouter_sommet(&g);

    arete a = {0,1,1};
    ajouter_arete(&g ,a);
    afficher(&g);

    free_graphe(&g);

    return EXIT_SUCCESS;
}