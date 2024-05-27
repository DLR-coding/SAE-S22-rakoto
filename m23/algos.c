#include "algos.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>

size_t degre(graphe const *g, sommet s)
{
    size_t degre = 0;
    int taille = g->nb_aretes + 1;
    sommet sa[taille]; // faire malloc
    degre = sommets_adjacents(g,s,sa);   
    return degre;
}

bool est_regulier(graphe const *g)
{
    bool test = true;
    size_t innitDegre = degre(g,g->aretes[0].s1);
    int i= 0;
    while (test == true && i < g->nb_aretes)
    {
        if (degre(g,g->aretes[i].s2) != innitDegre)
        {
            test = false;
        }   
        i++;
    }
    return test;
}

void afficher(graphe const *g)
{
    size_t nbsommets = g->ordre;
    printf("sommets = %d \n", nbsommets );
    printf("arêtes = %d \n", g->nb_aretes );
    printf("--SOMMETS-- \n");


    for (sommet i = 0; i < g->ordre; i++)
    {
        
    }
    

    
    //pr le 1er sommet
    size_t d;
    sommet sa = malloc(ordre * sizeof(sommet));
        sommet sa[t];
        d = sommets_adjacents(g,,sa);

    printf("%d (degré: %d) <-> ", 0 ,d);
    for (size_t i = 0; i < d; i++)
        {
            printf("%d",sa[i]);
        }
        printf("\n");

    //pr le reste des sommets 
    for (sommet i = 0; i < g->nb_aretes; i++)
    {
        size_t d;
        size_t taille = degre(g,g->aretes[i].s2);
        sommet sa[taille];
        d = sommets_adjacents(g,g->aretes[i].s2,sa);
        printf("%d (degré: %d) <->", i+1 , d);
        for (size_t i = 0; i < d; i++)
        {
            printf("%d",sa[i]);
        }
        printf("\n");
    }
    
    printf("--ARÊTES-- \n");

    /*
    0 - 2
    0 - 3
    1 - 4
    2 - 3
    3 - 4
    */
}

void generer_complet(graphe *g, size_t ordre)
{

}

void visite_composante_connexe(graphe const *g, sommet s, bool *visite)
{
}

uint32_t nb_composantes_connexes(graphe const *g)
{
    return 0;
}

bool sont_connectes(graphe const *g, sommet s1, sommet s2)
{
    return false;
}

void coloriage_glouton(graphe const *g, uint8_t *couleur_sommet)
{
}

void appliquer_permutation(graphe const *src, graphe *dst, size_t const *permutation)
{
}

uint32_t estimation_nb_chromatique(graphe const *g, uint32_t n)
{
    return 0;
}

void generer_aleatoire(graphe *g, size_t ordre, uint32_t k)
{
}

void dijkstra(graphe const *g, sommet s, double const *poids_arete, double *distance_sommet)
{
}

void trier_aretes(arete *aretes_triees, graphe const *g, double const *poids_arete)
{
}

void kruskal(graphe const *g, double const *poids_arete, graphe *acm)
{
}
