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
        printf("%ld - %ld : poid %.2f\n", a->s1, a->s2 , a->poids);
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
    // Initialiser toutes les couleurs à 255 (non-colorié)
    for (size_t i = 0; i < g->ordre; ++i) {
        couleur_sommet[i] = 255;
    }

    uint8_t couleur_max = 0;

    for (size_t s = 0; s < g->ordre; ++s) {
        // Créer un tableau pour marquer les couleurs utilisées par les voisins
        bool *couleurs_utilisees = calloc(couleur_max + 1, sizeof(bool));
        if (!couleurs_utilisees) {
            fprintf(stderr, "Erreur d'allocation de mémoire pour couleurs_utilisees\n");
            return;
        }

        // Marquer les couleurs utilisées par les voisins
        for (size_t i = 0; i < g->nb_aretes; ++i) {
            if (g->aretes[i].s1 == s && couleur_sommet[g->aretes[i].s2] != 255) {
                couleurs_utilisees[couleur_sommet[g->aretes[i].s2]] = true;
            } else if (g->aretes[i].s2 == s && couleur_sommet[g->aretes[i].s1] != 255) {
                couleurs_utilisees[couleur_sommet[g->aretes[i].s1]] = true;
            }
        }

        // Trouver la plus petite couleur non utilisée
        uint8_t couleur = 0;
        while (couleur <= couleur_max && couleurs_utilisees[couleur]) {
            ++couleur;
        }

        // Assigner cette couleur au sommet
        couleur_sommet[s] = couleur;

        // Mettre à jour la couleur maximale utilisée
        if (couleur == couleur_max + 1) {
            ++couleur_max;
        }

        free(couleurs_utilisees);
    }
}


void appliquer_permutation(graphe const *src, graphe *dst, size_t const *permutation) 
{ 
 for (size_t i=0;i<src->ordre;i++){ 
  ajouter_sommet(dst);  
 } 
   for (size_t i = 0; i < src->nb_aretes; ++i) { 
       sommet nouveau_s1 = permutation[src->aretes[i].s1]; 
       sommet nouveau_s2 = permutation[src->aretes[i].s2]; 
       arete a = {nouveau_s1, nouveau_s2, src->aretes[i].poids}; 
       ajouter_arete(dst, a); 
 } 
}

uint32_t estimation_nb_chromatique(graphe const *g, uint32_t n) 
{ /*
   uint32_t min_couleurs = g->ordre;  // Initialiser à un maximum possible de couleurs 
   size_t *permutation = malloc(g->ordre * sizeof(size_t)); 
   uint8_t *couleur_sommet = malloc(g->ordre * sizeof(uint8_t)); 
   graphe g_permuted; 

   for (uint32_t i = 0; i < n; i++) { 
       generer_permutation(permutation, g->ordre); 
       appliquer_permutation(g, &g_permuted, permutation); 

       coloriage_glouton(&g_permuted, couleur_sommet); 

       uint8_t max_couleur = 0; 
       for (size_t j = 0; j < g->ordre; j++) { 
           if (couleur_sommet[j] > max_couleur) { 
               max_couleur = couleur_sommet[j]; 
           } 
       } 

       if (max_couleur + 1 < min_couleurs) { 
           min_couleurs = max_couleur + 1; 
       } 

       free(g_permuted.aretes); 
   } 

   free(permutation); 
   free(couleur_sommet); 

   return min_couleurs; */
} 

void generer_aleatoire(graphe *g, size_t ordre, uint32_t k) 
{ 
 for (size_t i = 0;i<ordre; i++){ 
   ajouter_sommet(g); 
 } 

   for (size_t i = 0; i < ordre; ++i) { 
       for (size_t j = i + 1; j < ordre; ++j) { 
           if (rand() % k == 0) { 
               arete a = {i, j}; 
               ajouter_arete(g, a); 
           } 
       } 
   } 
}
void dijkstra(graphe const *g, sommet s, double const *poids_arete, double *distance_sommet) 
{ 
 bool* visited = calloc(g->ordre, sizeof(bool)); 
   for (size_t i = 0; i < g->ordre; ++i) { 
       distance_sommet[i] = DBL_MAX; 
   } 
   distance_sommet[s] = 0; 

   for (size_t i = 0; i < g->ordre; ++i) { 
       double min_distance = DBL_MAX; 
       size_t u = -1; 
       for (size_t j = 0; j < g->ordre; ++j) { 
           if (!visited[j] && distance_sommet[j] < min_distance) { 
               min_distance = distance_sommet[j]; 
               u = j; 
           } 
       } 

       if (u == -1) break; 

       visited[u] = true; 
       for (size_t j = 0; j < g->nb_aretes; ++j) { 
           if (g->aretes[j].s1 == u || g->aretes[j].s2 == u) { 
               size_t v = (g->aretes[j].s1 == u) ? g->aretes[j].s2 : g->aretes[j].s1; 
               if (!visited[v] && distance_sommet[u] + poids_arete[j] < distance_sommet[v]) { 
                   distance_sommet[v] = distance_sommet[u] + poids_arete[j]; 
               } 
           } 
       } 
   } 

   free(visited); 
}

void trier_aretes(arete *aretes_triees, graphe const *g, double const *poids_arete) 
{ 
   // Copier les arêtes dans aretes_triees 
   for (size_t i = 0; i < g->nb_aretes; ++i) { 
       aretes_triees[i] = g->aretes[i]; 
   } 

   // Tableau auxiliaire pour les poids des arêtes 
   double *poids_aux = malloc(g->nb_aretes * sizeof(double)); 
   for (size_t i = 0; i < g->nb_aretes; ++i) { 
       poids_aux[i] = poids_arete[i]; 
   } 

   // Tri par insertion basé sur les poids des arêtes 
   for (size_t i = 1; i < g->nb_aretes; ++i) { 
       arete key = aretes_triees[i]; 
       double key_weight = poids_aux[i]; 
       size_t j = i; 
       while (j > 0 && poids_aux[j - 1] > key_weight) { 
           aretes_triees[j] = aretes_triees[j - 1]; 
           poids_aux[j] = poids_aux[j - 1]; 
           --j; 
       } 
       aretes_triees[j] = key; 
       poids_aux[j] = key_weight; 
   } 

   // Libérer la mémoire allouée 
   free(poids_aux); 
}

void kruskal(graphe const *g, double const *poids_arete, graphe *acm)
{
}
