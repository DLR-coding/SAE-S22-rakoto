#include "algos.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>

size_t degre(graphe const *g, sommet s)
{
    size_t degre = 0;
    int taille = g->nb_aretes;
    sommet *sa = malloc(taille * sizeof(sommet)); 
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
    printf("sommets = %ld \n", nbsommets );
    printf("arêtes = %d \n", g->nb_aretes );
    printf("--SOMMETS-- \n");

    // pour chaque sommet, on affiche ses voisins
    for (sommet i = 0; i < g->ordre; i++) {
        printf("%d (degré: %d) <-> ", i, degre(g, i));
        sommet *sa = malloc(g->ordre * sizeof(sommet));
        size_t d = sommets_adjacents(g, i, sa);
        for (size_t j = 0; j < d; j++) 
        {
            printf("%d ", sa[j]);
        }
        free(sa);
        printf("\n");
    }

    printf("--ARÊTES-- \n");

    // pour chaque arête, on affiche ses deux sommets
    for (size_t i = 0; i < g->nb_aretes; i++) {
        arete const *a = &(g->aretes[i]);
        printf("%ld - %ld\n", a->s1, a->s2);
    }
}

void generer_complet(graphe *g, size_t ordre)
{
    g->ordre = ordre;
    for (sommet i = 0; i < ordre; i++)
    {
        for (sommet j = i+1; j < ordre; j++)
        {
            
               arete a = {
                    .s1 = i ,
                    .s2 = j ,
                };
                ajouter_arete(g,a);  
                                       
        }
    }
    
}

void visite_composante_connexe(graphe const *g, sommet s, bool *visite)
{
    visite[s] = true; // on marque le sommet s comme étant visité

    sommet sa[g->ordre];
    size_t nb_sommets_adjacents = sommets_adjacents(g, s, sa);
    // on parcourt tous les sommets adjacents à s
    for (size_t i = 0; i < nb_sommets_adjacents; i++)
    {
        sommet voisin = sa[i];
        if (!visite[voisin])
        {
            visite_composante_connexe(g, voisin, visite);
        }
    }
}

uint32_t nb_composantes_connexes(graphe const *g)
{
    size_t nb_composantes = 0;
    // on alloue un tableau de booléens pour matérialiser le fait qu'un sommet a été visité ou non
    bool *visite = malloc(g->ordre * sizeof(bool));
    for (size_t i = 0; i < g->ordre; i++)
    {
        visite[i] = false; // on initialise toutes les cases à false
    }

    // on parcourt tous les sommets du graphe
    for (sommet s = 0; s < g->ordre; s++)
    {
        if (!visite[s]) // si le sommet s n'a pas encore été visité
        {
            visite_composante_connexe(g, s, visite); // on explore la composante connexe contenant s
            nb_composantes++; // et on incrémente le nombre de composantes connexes
        }
    }

    free(visite); // on libère la mémoire allouée dynamiquement

    return nb_composantes;
}

bool sont_connectes(graphe const *g, sommet s1, sommet s2)
{
    // initialiser un tableau de booléens pour garder une trace des sommets visités
    bool *visite = malloc(g->ordre * sizeof(bool));
    for (size_t i = 0; i < g->ordre; i++)
    {
        visite[i] = false;
    }

    // initialiser une file d'attente pour le parcours en largeur
    sommet file[g->ordre];
    int debut = 0;
    int fin = 0;

    // marquer le sommet s1 comme visité et l'ajouter à la file d'attente
    visite[s1] = true;
    file[fin++] = s1;

    // parcourir la file d'attente jusqu'à ce qu'elle soit vide ou que l'on ait trouvé s2
    while (debut < fin)
    {
        // récupérer le sommet actuel à partir du début de la file d'attente
        sommet s = file[debut++];

        // vérifier si le sommet actuel est s2
        if (s == s2)
        {
            // libérer la mémoire allouée pour le tableau de visites
            free(visite);

            // renvoyer true pour indiquer que les sommets sont connectés
            return true;
        }

        // parcourir tous les sommets adjacents à s
        sommet sa[g->ordre];
        size_t nb_sommets_adjacents = sommets_adjacents(g, s, sa);
        for (size_t i = 0; i < nb_sommets_adjacents; i++)
        {
            sommet voisin = sa[i];

            // vérifier si le sommet voisin n'a pas déjà été visité
            if (!visite[voisin])
            {
                // marquer le sommet voisin comme visité et l'ajouter à la file d'attente
                visite[voisin] = true;
                file[fin++] = voisin;
            }
        }
    }

    // libérer la mémoire allouée pour le tableau de visites
    free(visite);

    // renvoyer false pour indiquer que les sommets ne sont pas connectés
    return false;
}

void coloriage_glouton(graphe const *g, uint8_t *couleur_sommet)
{
    // initialisation
    size_t nb_sommets = g->ordre;
    int max = 255;
    uint8_t couleur_max = 0;
    for (size_t i = 0; i < nb_sommets; i++) {
        couleur_sommet[i] = max; // marqueur pour "pas encore colorié"
    }

    // traitement des sommets dans l'ordre
    for (sommet s = 0; s < nb_sommets; s++) {
        if (couleur_sommet[s] == max) { 
            // if sommet pas encore colorié
            // cherche de la plus petite couleur disponible
            bool *colors = malloc(sizeof(bool) * (couleur_max + 1));
            for (size_t i = 0; i <= couleur_max; i++) {
                colors[i] = false;
            }
            sommet sa[nb_sommets];
            size_t nb_voisins = sommets_adjacents(g, s, sa);
            for (size_t i = 0; i < nb_voisins; i++) //met les voisins déjà colorié à true
            {
                sommet voisin = sa[i];
                if (couleur_sommet[voisin] != max) { // si le voisin est déjà colorié
                    colors[couleur_sommet[voisin]] = true;
                }
            }
            size_t couleur_s = 0;
            while (colors[couleur_s]) {
                couleur_s++;
            }
            if (couleur_s > couleur_max) {
                couleur_max = couleur_s;
            }
            couleur_sommet[s] = couleur_s;

            free(colors);
        }
    }
    
    
}

void appliquer_permutation(graphe const *src, graphe *dst, size_t const *permutation)
{
    /*
    dst->ordre = src->ordre;
    arete a = {
        .s1 = src->aretes[i],
        .s2 = src->aretes[i]
    };
    dst->
    */

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
