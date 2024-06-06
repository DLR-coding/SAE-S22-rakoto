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
    uint8_t data[1500];
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


//Fonction pour convertir bits en adresse MAC
void bitsToMACAddress(uint8_t bits[48], MACAddress *mac) {
    for (int i = 0; i < 6; i++) {
        mac->octets[i] = 0;
        for (int j = 0; j < 8; j++) {
            mac->octets[i] <<= 1;
            mac->octets[i] |= bits[i * 8 + j];
        }
    }
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

bool reseau_ajouter_connexion(reseau *r, size_t indice_eqt1, size_t indice_eqt2) {
    // Vérifier que les indices des équipements sont valides
    if (indice_eqt1 >= r->nb_equipements || indice_eqt2 >= r->nb_equipements) {
        return false;
    }

    // Vérifier que les deux équipements sont de types différents (une station ne peut pas être connectée à une autre station mais 2 switch peuvent)
    if ((r->m_equipements[indice_eqt1].type == STATION) && (r->m_equipements[indice_eqt1].type == r->m_equipements[indice_eqt2].type)) {
        return false;
    }

    // Créer une nouvelle arête entre les deux équipements
    arete a = { .s1 = indice_eqt1, .s2 = indice_eqt2 };
    bool res = ajouter_arete(&r->m_graphe, a);
    return res;
}

// Print un reseau
void printReseau(reseau r)
{
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
            printMACAddress(r.m_equipements[i].data.m_switch.mac);
            printf("  Nombre de ports : %d\n", r.m_equipements[i].data.m_switch.nbport);
            printf("  Priorité : %d\n", r.m_equipements[i].data.m_switch.priorite);
            printf("  Table de commutation :\n");
            printSwitchTable(&r.m_equipements[i].data.m_switch);
        }
    }
    printf("\n");
    afficher(&r.m_graphe);
       
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
    graphe gReseau;
    init_graphe(&gReseau);
    r->m_graphe = gReseau;

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

        arete a = { .s1 = indice_eqt1, .s2 = indice_eqt2 };
        ajouter_arete(&r->m_graphe, a);
    }

    fclose(fp);
    return true;
}

///////// Partie 3

// transfer de trame : 


//parcourt la table de commutation du switch 
///et vérifie si l'adresse MAC de destination correspond à une entrée dans la table. 
//Si c'est le cas, elle retourne le numéro de port correspondant à l'entrée. 
//Si l'adresse MAC de destination ne se trouve pas dans la table, elle retourne -1

int trouver_port_station(Switch *sw, MACAddress mac_dest) {
    // Parcourir la table de commutation du switch
    for (int i = 0; i < sw->table.nb_entrees; i++) {
        switchTableEntry *entree = &(sw->table.entrees[i]);
        // Vérifier si l'adresse MAC de destination correspond à entree 
        if (memcmp(entree->m_macAddress, mac_dest, 6) == 0) {
            // Retourner le numéro de port correspondant à l'entrée
            return entree->m_port;
        }
    }
    // Si l'adresse MAC de destination ne se trouve pas dans la table, retourner -1
    return -1;
}

//switch envoyer tram à switch. utiliser quand switch --> equipement
void envoyer_trame(Switch* sw, trame_ethernet* trame, int port_dest) 
{
    // Vérifier que le port de destination est valide
    if (port_dest < 1 || port_dest > sw->nbport) {
        printf("Erreur : le port de destination %d est invalide\n", port_dest);
        return;
    }

    // Envoyer la trame sur le port de destination
    printf("Envoi de la trame sur le port %d :\n", port_dest);
    afficher_trame_ethernet(*trame);

    
    // Mettre à jour la table de commutation du switch si nécessaire
    MACAddress mac_source = trame->source_mac;
    MACAddress mac_dest = trame->destination_mac;
    int port_source = trouver_port_station(sw, mac_source);
    if (port_source != -1) {
        // La trame a été envoyée par une station connectée au switch, mettre à jour la table de commutation
        addSwitchTableEntry(sw, mac_dest, port_dest);
    } 
    
}

//switch recois trame d'une station. utiliser quand stations --> switch
void recevoirTrame(Switch* sw, trame_ethernet* trame, int port) {
    printf("trame recu au port %d\n", port);
    // Vérifier si la trame est destinée à une station connectée au switch
    MACAddress mac_dest = trame->destination_mac;
    int port_dest = trouver_port_station(sw, mac_dest); //trouver le port qui mène au MAC destinataire (-1 s'il trv pas)

    if (port_dest != -1) {
        // transferer la trame vers le port de la station destinataire
        printf("Réémission de la trame sur le port %d => vers la station destinataire\n", port_dest);
        envoyer_trame(sw, trame, port_dest);
    } else {
        // BROADCAST. sauf celui d'où elle a été reçue
        printf("BROADCAST trame sur tous les ports sauf le port %d\n", port);
        for (int i = 1; i <= sw->nbport; i++) {
            if (i != port) {
                envoyer_trame(sw, trame, i);
            }
        }
    }

    // Mettre à jour la table de commutation du switch
    MACAddress mac_src = trame->source_mac;
    addSwitchTableEntry(sw, mac_src, port);
}



/// Partie 4
bool stp(reseau *r)
{
    un switch a nbports : 
        switch 1: 
            ports[nbports]
            etat_ports[nbports]
            port[1] --> etat_ports[1]            
    
   
    // election switch racine
    for (size_t i = 0; i < r->nb_equipements; i++)
    {
        
    }
    
   return false;
}
int main() {
    /*
    //PARTIE 2    
    // Créer une structure de réseau vide
    reseau r;

    // Initialiser le réseau
    init_reseau(&r);

    // Lire le fichier de configuration et créer le réseau
    if (!lire_config("reseau_config.txt", &r)) {
        fprintf(stderr, "Erreur lors de la lecture du fichier de configuration.\n");
        return 1;
    }

    // Afficher les informations du réseau
    printf("Affichage du réseau : \n");
    printReseau(r);

    // Libérer la mémoire allouée pour les équipements et la structure de graphe
    free(r.m_equipements);
    free_graphe(&r.m_graphe);
    */

   ""


    return 0;
    
}
