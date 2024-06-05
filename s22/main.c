#include "s22.h"
#include "./graphe.h"
#include "./algos.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


int main() {
    /*
    // Créer une structure de réseau vide
    reseau r = {0};

    // Lire le fichier de configuration et créer le réseau
    lire_config("config.txt", &r);

    // Afficher les informations du réseau
    printf("Nombre d'équipements : %zu\n", r.nb_equipements);
    for (size_t i = 0; i < r.nb_equipements; i++) {
        printf("Équipement %zu :\n", i+1);
        if (r.m_equipements[i].type == STATION) {
            printf("  Type : Station\n");
            printf("  Adresse MAC : ");
            printMACAddress(r.m_equipements[i].data.m_station.m_adresseMac);
            printf("  Adresse IP : ");
            printIPAddress(r.m_equipements[i].data.m_station.m_adresseIP);
            printf("\n");
        } else if (r.m_equipements[i].type == SWITCH) {
            printf("  Type : Switch\n");
            printf("  Adresse MAC : ");
            printMACAddress(r.m_equipements[i].data.m_switch.MAC);
            printf("  Nombre de ports : %d\n", r.m_equipements[i].data.m_switch.nbport);
            printf("  Priorité : %d\n", r.m_equipements[i].data.m_switch.priorite);
            printf("  Table de commutation :\n");
            printSwitchTable(&r.m_equipements[i].data.m_switch, r.m_equipements[i].data.m_switch.nbport);
        }
    }

    // Libérer la mémoire allouée pour les équipements et la structure de graphe
    free(r.m_equipements);
    free_graphe(&r.m_graphe);
        

       // Créer un switch avec 4 ports et une capacité de table de commutation de 2 entrées
    MACAddress mac = {{0x00, 0x11, 0x22, 0x33, 0x44, 0x55}};
    Switch sw;
    initSwitch(&sw, mac, 4, 32, 2);

    // Ajouter des entrées à la table de commutation du switch
    MACAddress mac1 = {{0x00, 0x01, 0x02, 0x03, 0x04, 0x05}};
    addSwitchTableEntry(&sw, mac1, 1);
    MACAddress mac2 = {{0x00, 0x06, 0x07, 0x08, 0x09, 0x0a}};
    addSwitchTableEntry(&sw, mac2, 2);
    MACAddress mac3 = {{0x00, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f}};
    addSwitchTableEntry(&sw, mac3, 3);

    // Afficher la table de commutation du switch
    printSwitchTable(&sw);
*/
    srand(time(NULL));

    graphe g;
    init_graphe(&g);

    printf("AFFICHER GRAPHE\n");
    printf("===============\n");
    ajouter_sommet(&g);
    ajouter_sommet(&g);

    arete a = {0,1};
    ajouter_arete(&g ,a);
    afficher(&g);

    free_graphe(&g);

    return EXIT_SUCCESS;
}