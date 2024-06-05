#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "graphe.h"
#include "algos.h"


//////////// STRUCTURES ////////////////

// Structure pour stocker une adresse IP
typedef struct IPAddress{
    uint8_t octet[4];
} IPAddress;

typedef struct MACAddress{
    uint8_t octets[6];
} MACAddress;

// Dans tablecommutation on fait un tab d'entrees
typedef struct switchTableEntry {
    MACAddress m_macAddress;
    int m_port;
} switchTableEntry;

typedef struct TableDeCommutation {
    int capacite; // Pour avoir un max de lignes dedans , ou alors dans le Entry on fait un { *mac_addressTAB , int port}
    int nb_entrees;
    switchTableEntry *entrees;
} TableDeCommutation;


// switch
typedef struct Switch
{
    MACAddress mac;
    int nbport;
    int priorite;
    TableDeCommutation table; 
} Switch; 

// Structure de la station
typedef struct station{
    IPAddress m_adresseIP;
    MACAddress m_adresseMac;
}station;


typedef struct equipement {
    enum type_equipement { STATION, SWITCH } type;
    union {
        station m_station;
        Switch m_switch;
    } data;
} equipement;

typedef struct reseau {
    graphe m_graphe;
    equipement* m_equipements;
    size_t nb_equipements;
} reseau;

typedef struct trame_ethernet{
    uint8_t preambule[7];
    uint8_t sfd;
    MACAddress destination_mac;
    MACAddress source_mac;
    uint16_t type;
    uint8_t data[1500]; //uint8_t* data ?
    uint32_t fcs;
} trame_ethernet;


///////////////////////////

// pour crer un switch 
void initSwitch(Switch *sw, MACAddress MAC, int nbport, int priorite, int capacite) {
    sw->mac = MAC;
    sw->nbport = nbport;
    sw->priorite = priorite;

    // Initialiser la table de commutation
    sw->table.capacite = capacite;
    sw->table.nb_entrees = 0;
    sw->table.entrees = malloc(capacite * sizeof(switchTableEntry));
    if (sw->table.entrees == NULL) {
        // Gérer l'erreur d'allocation mémoire
        exit(EXIT_FAILURE);
    }
}

//pr ajouter une entrée
void addSwitchTableEntry(Switch *sw, MACAddress mac, int port) {
    // inspiré de reallloc dans les graphes
    if (sw->table.nb_entrees == sw->table.capacite) {
        // Doubler la capacité de la table de commutation
        sw->table.capacite *= 2;
        sw->table.entrees = realloc(sw->table.entrees, sw->table.capacite * sizeof(switchTableEntry));
        if (sw->table.entrees == NULL) {
            // Gérer l'erreur d'allocation mémoire
            exit(EXIT_FAILURE);
        }
    }

    // Ajouter l'entrée à la table de commutation
    sw->table.entrees[sw->table.nb_entrees].m_macAddress = mac;
    sw->table.entrees[sw->table.nb_entrees].m_port = port;
    sw->table.nb_entrees++;
}

void init_reseau(reseau *r) {
    // Initialiser le graphe
    init_graphe(&r->m_graphe);

    // Initialiser le tableau d'équipements
    r->m_equipements = NULL;
    r->nb_equipements = 0;
}


// Fonction pour afficher une adresse IP
void printIPAddress(IPAddress ip) {
    printf("%d.%d.%d.%d", ip.octet[0], ip.octet[1],ip.octet[2], ip.octet[3]);
}


// Fonction pour afficher une adresse Mac
void printMACAddress(MACAddress mac) {
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac.octets[0], mac.octets[1], mac.octets[2],
                                                mac.octets[3], mac.octets[4], mac.octets[5]);
}


// Fonction pour afficher la table de commutation d'un switch
void printSwitchTable(Switch *sw) {
    printf("Table de commutation du switch %02X:%02X:%02X:%02X:%02X:%02X\n",
           sw->mac.octets[0], sw->mac.octets[1], sw->mac.octets[2],
           sw->mac.octets[3], sw->mac.octets[4], sw->mac.octets[5]);
    for (int i = 0; i < sw->table.nb_entrees; i++) {
        printf("%d: %02X:%02X:%02X:%02X:%02X:%02X -> Port %d\n",
               i+1,
               sw->table.entrees[i].m_macAddress.octets[0],
               sw->table.entrees[i].m_macAddress.octets[1],
               sw->table.entrees[i].m_macAddress.octets[2],
               sw->table.entrees[i].m_macAddress.octets[3],
               sw->table.entrees[i].m_macAddress.octets[4],
               sw->table.entrees[i].m_macAddress.octets[5],
               sw->table.entrees[i].m_port);
    }
}


// Print un reseau
void printReseau(const reseau *r)
{
        printf("Réseau :\n");
    printf("Nombre d'équipements : %zu\n", r->nb_equipements);
    printf("Nombre de liens : %zu\n", nb_aretes(&r->m_graphe));

    printf("\nÉquipements :\n");
    for (size_t i = 0; i < r->nb_equipements; i++) {
        if (r->m_equipements[i].type == SWITCH) {
            printf("Switch sw%zu\n", i + 1);
            printf("— Adresse MAC : %02x:%02x:%02x:%02x:%02x:%02x\n",
                   r->m_equipements[i].data.m_switch.mac.octets[0],
                   r->m_equipements[i].data.m_switch.mac.octets[1],
                   r->m_equipements[i].data.m_switch.mac.octets[2],
                   r->m_equipements[i].data.m_switch.mac.octets[3],
                   r->m_equipements[i].data.m_switch.mac.octets[4],
                   r->m_equipements[i].data.m_switch.mac.octets[5]);
            printf("— Nombre de ports : %d\n", r->m_equipements[i].data.m_switch.nbport);
            printf("— Priorité : %d\n", r->m_equipements[i].data.m_switch.priorite);
        } else if (r->m_equipements[i].type == STATION) {
            printf("Station st%zu\n", i + 1);
            printf("— Adresse IP : %d.%d.%d.%d\n",
                   r->m_equipements[i].data.m_station.m_adresseIP.octet[0],
                   r->m_equipements[i].data.m_station.m_adresseIP.octet[1],
                   r->m_equipements[i].data.m_station.m_adresseIP.octet[2],
                   r->m_equipements[i].data.m_station.m_adresseIP.octet[3]);
            printf("— Adresse MAC : %02x:%02x:%02x:%02x:%02x:%02x\n",
                   r->m_equipements[i].data.m_station.m_adresseMac.octets[0],
                   r->m_equipements[i].data.m_station.m_adresseMac.octets[1],
                   r->m_equipements[i].data.m_station.m_adresseMac.octets[2],
                   r->m_equipements[i].data.m_station.m_adresseMac.octets[3],
                   r->m_equipements[i].data.m_station.m_adresseMac.octets[4],
                   r->m_equipements[i].data.m_station.m_adresseMac.octets[5]);
        }
    }

    printf("\nGraphe :\n");
    afficher(&r->m_graphe);
}



//Partie 2
bool lire_config(const char *nomFichier, reseau *r) {
    FILE *fp = fopen(nomFichier, "r");
    if (fp == NULL) {
        printf("Erreur : impossible d'ouvrir le fichier de configuration\n");
        return false;
    }

    int num_equipements;
    int num_liens;
    fscanf(fp, "%d %d\n", &num_equipements, &num_liens);

    r->nb_equipements = num_equipements;
    r->m_equipements = malloc(num_equipements * sizeof(equipement));
    if (r->m_equipements == NULL) {
        printf("Erreur : impossible d'allouer de la mémoire pour les équipements\n");
        fclose(fp);
        return false;
    }

    init_graphe(&r->m_graphe);

    for (int i = 0; i < num_equipements; i++) {
        int type;
        fscanf(fp, "%d;", &type);

        if (type == 1) {
            MACAddress mac;
            IPAddress ip;
            fscanf(fp, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx;", &mac.octets[0], &mac.octets[1], &mac.octets[2], &mac.octets[3], &mac.octets[4], &mac.octets[5]);
            fscanf(fp, "%hhu.%hhu.%hhu.%hhu\n", &ip.octet[0], &ip.octet[1], &ip.octet[2], &ip.octet[3]);

            equipement e;
            e.type = STATION;
            e.data.m_station.m_adresseMac = mac;
            e.data.m_station.m_adresseIP = ip;
            r->m_equipements[i] = e;
            ajouter_sommet(&r->m_graphe);

        } else if (type == 2) {
            MACAddress mac;
            int nbport, priorite;
            fscanf(fp, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx;", &mac.octets[0], &mac.octets[1], &mac.octets[2], &mac.octets[3], &mac.octets[4], &mac.octets[5]);
            fscanf(fp, "%d;", &nbport);
            fscanf(fp, "%d\n", &priorite);

            equipement e;
            e.type = SWITCH;
            initSwitch(&e.data.m_switch, mac, nbport, priorite, nbport);
            r->m_equipements[i] = e;
            ajouter_sommet(&r->m_graphe);

        } else {
            printf("Erreur : type d'équipement inconnu\n");
            fclose(fp);
            return false;
        }
    }

    for (int i = 0; i < num_liens; i++) {
        int indice_eqt1, indice_eqt2, cout;
        fscanf(fp, "%d;%d;%d\n", &indice_eqt1, &indice_eqt2, &cout);

        if (indice_eqt1 >= num_equipements || indice_eqt2 >= num_equipements) {
            printf("Erreur : indice d'équipement invalide\n");
            fclose(fp);
            return false;
        }
        arete a = {indice_eqt1,indice_eqt2};
        ajouter_arete(&r->m_graphe, a);
    }

    fclose(fp);
    return true;
}

void free_reseau(reseau *r) {
    // Libérer la mémoire allouée pour le tableau d'équipements
    for (size_t i = 0; i < r->nb_equipements; i++) {
        if (r->m_equipements[i].type == SWITCH) {
            free(r->m_equipements[i].data.m_switch.table.entrees);
        }
    }
    free(r->m_equipements);

    // Libérer la mémoire allouée pour le graphe
    free_graphe(&r->m_graphe);
}

int main() {

    //PARTIE 2    
    // Créer une structure de réseau vide
    reseau r;
    init_reseau(&r);

    lire_config("reseau_config.txt", &r);
    printReseau(&r);

    free_reseau(&r);

    return 0;
    
}
