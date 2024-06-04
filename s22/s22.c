#include <stdio.h>
#include <stdint.h>
#include "graph.h"


//////////// STRUCTURES ////////////////

// Structure pour stocker une adresse IP
typedef struct IPAddress{
    uint8_t octet[4];
} IPAddress;

typedef struct MACAddress{
    uint8_t octets[6];
} MACAddress;

typedef struct switchTableEntry {
    MACAddress m_macAddress;
    int m_port;
} switchTableEntry;

typedef struct TableDeCommutation {
    int capacite;
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


// Fonction pour afficher une adresse IP
void printIPAddress(IPAddress ip) {
    printf("%d.%d.%d.%d", ip.octet[0], ip.octet[1], ip.octet[2], ip.octet[3]);
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

    // Si l'arête a été ajoutée avec succès, mettre à jour les tables de commutation des switchs connectés
    if (res) {
        if (r->m_equipements[indice_eqt1].type == SWITCH) {
            addSwitchTableEntry(&r->m_equipements[indice_eqt1].data.m_switch, r->m_equipements[indice_eqt2].data.m_station.m_adresseMac ,-1);
        }
        if (r->m_equipements[indice_eqt2].type == SWITCH) {
            addSwitchTableEntry(&r->m_equipements[indice_eqt2].data.m_switch, r->m_equipements[indice_eqt1].data.m_station.m_adresseMac, -1);
        }
    }

    return res;
}

// Fonction pour calculer les informations sur le réseau
void addnet(IPAddress ip, IPAddress mask) 
{
    // Calcul du masque de bits
    uint32_t subnetMask = (mask.octet[0] << 24) | (mask.octet[1] << 16) | (mask.octet[2] << 8) | mask.octet[3];
    
    // Adresse réseau = IP & masque de sous-réseau
    uint32_t networkAddress = (ip.octet[0] << 24) | (ip.octet[1] << 16) | (ip.octet[2] << 8) | ip.octet[3];
    networkAddress &= subnetMask;

    // Adresse de broadcast = adresse réseau | inverse du masque
    uint32_t broadcastAddress = networkAddress | ~subnetMask;

    // Nombre d'adresses adressables = (2^(nombre de bits dans l'adresse) - 2)
    uint32_t numHosts = (~subnetMask) - 1;
        //test
    // Affichage des résultats
    printf("Classe du reseau: ");
    if (ip.octet[0] < 128) {
        printf("A\n");
    } else if (ip.octet[0] < 192) {
        printf("B\n");
    } else if (ip.octet[0] < 224) {
        printf("C\n");
    } else if (ip.octet[0] < 240) {
        printf("D (multicast)\n");
    } else {
        printf("E (expérimentale)\n");
    }

    printf("Adresse du reseau: ");
    printIPAddress((IPAddress){.octet = {networkAddress >> 24, (networkAddress >> 16) & 0xFF, (networkAddress >> 8) & 0xFF, networkAddress & 0xFF}});
    printf("\n");

    printf("Adresse de la station: ");
    printIPAddress(ip);
    printf("\n");

    printf("Adresse de broadcast: ");
    printIPAddress((IPAddress){.octet = {broadcastAddress >> 24, (broadcastAddress >> 16) & 0xFF, (broadcastAddress >> 8) & 0xFF, broadcastAddress & 0xFF}});
    printf("\n");

    printf("Nombre de machines adressables dans le sous-reseau: %u\n", numHosts);
}

//Partie 2
bool lire_config(const char *nomFichier, reseau *r) 
{
    
    // Ouvrir le fichier de configuration
    FILE* fp = fopen(nomFichier, "r");
    if (fp == NULL) {
        printf("Erreur : impossible d'ouvrir le fichier de configuration\n");
        return false;
    }

    // Lire le nombre d'équipements et le nombre de liens
    int num_equipements;
    int num_liens;
    fscanf(fp, "%d %d\n", &num_equipements, &num_liens);

    // Initialiser le réseau
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

    // Lire les informations des équipements
    for (int i = 0; i < num_equipements; i++) {
        int type;
        fscanf(fp, "%d;", &type);

        if (type == 1) {
            // Lire les informations de la station
            MACAddress mac;
            IPAddress ip;
            fscanf(fp, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx;", &mac.octets[0], &mac.octets[1], &mac.octets[2], &mac.octets[3], &mac.octets[4], &mac.octets[5]);
            fscanf(fp, "%hhu.%hhu.%hhu.%hhu\n", &ip.octet[0], &ip.octet[1], &ip.octet[2], &ip.octet[3]);

            // Créer la station et l'ajouter au réseau
            equipement e;
            e.type = STATION;
            e.data.m_station.m_adresseMac = mac;
            e.data.m_station.m_adresseIP = ip;
            r->m_equipements[i] = e;
            ajouter_sommet(&r->m_graphe);

        } else if (type == 2) {
            // Lire les informations du switch
            MACAddress mac;
            int nbport, priorite;
            fscanf(fp, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx;", &mac.octets[0], &mac.octets[1], &mac.octets[2], &mac.octets[3], &mac.octets[4], &mac.octets[5]);
            fscanf(fp, "%d;", &nbport);
            fscanf(fp, "%d\n", &priorite);

            // Créer le switch et l'ajouter au réseau
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

    // Lire les informations des liens
    for (int i = 0; i < num_liens; i++) {
        int indice_eqt1, indice_eqt2, cout;
        fscanf(fp, "%d;%d;%d\n", &indice_eqt1, &indice_eqt2, &cout);

        // Ajouter le lien au réseau
        arete a  = {
            .s1 = indice_eqt1,
            .s2 = indice_eqt2
        };
        ajouter_arete(&r->m_graphe,a);

        // Mettre à jour les tables de commutation des switchs connectés (si c un switch vers un autre switch jsp si cv marcher )
        if (r->m_equipements[indice_eqt1].type == SWITCH) {
            addSwitchTableEntry(&r->m_equipements[indice_eqt1].data.m_switch, r->m_equipements[indice_eqt2].data.m_station.m_adresseMac, indice_eqt2);
        }
        if (r->m_equipements[indice_eqt2].type == SWITCH) {
            addSwitchTableEntry(&r->m_equipements[indice_eqt2].data.m_switch, r->m_equipements[indice_eqt1].data.m_station.m_adresseMac, indice_eqt1);
        }
    }

    // Fermer le fichier de configuration
    fclose(fp);

    // Retourner vrai pour indiquer que la lecture du fichier de configuration a réussi
    return true;
}

//////////////:
/////////// Partie 3
/*
void transfert_trame(equipement *eqt1, equipement *eqt2, int port1, int port2) {
    // Création d'une trame Ethernet pour un échange entre eqt1 et eqt2
    trame_ethernet trame;
    memset(trame.preamble, 0xaa, 7);
    trame.preamble[6] = 0xab;
    trame.sfd = 0xab;
    memcpy(trame.destination_mac.octets, eqt2->data.m_station.m_adresseMac.octets, 6);
    memcpy(trame.source_mac.octets, eqt1->data.m_station.m_adresseMac.octets, 6);
    trame.type = 0x0800; // Type de la trame : IPv4
    memcpy(trame.data, "Hello, world!", 13); // Données de la trame : "Hello, world!"
    trame.fcs = 0; // FCS de la trame : à calculer ultérieurement

    // Envoi de la trame Ethernet de eqt1 à eqt2 via les ports spécifiés
    printf("Envoi de la trame Ethernet de %s à %s via les ports %d et %d :\n",
           eqt1->data.m_station.m_adresseMac.octets, eqt2->data.m_station.m_adresseMac.octets,
           port1, port2);
    afficher_trame_ethernet(trame);
    recevoirTrame(eqt2, &trame, port2);

    // Réémission de la trame Ethernet de eqt2 à eqt1 via les ports spécifiés
    printf("\nRéémission de la trame Ethernet de %s à %s via les ports %d et %d :\n",
           eqt2->data.m_station.m_adresseMac.octets, eqt1->data.m_station.m_adresseMac.octets,
           port2, port1);
    afficher_trame_ethernet(trame);
    recevoirTrame(eqt1, &trame, port1);

    // Affichage de la réception de la trame Ethernet par eqt1
    printf("\n%s a reçu la trame Ethernet de %s avec les données suivantes : ",
           eqt1->data.m_station.m_adresseMac.octets, eqt2->data.m_station.m_adresseMac.octets);
    for (int i = 0; i < 13; i++) {
        printf("%c", trame.data[i]);
    }
    printf("\n");
} 
*/
void print_ethernet_trame(trame_ethernet *trame, bool hex_mode) {
    if (hex_mode) {
        // Affichage en hexadécimal
        printf("Preambule: ");
        for (int i = 0; i < 7; i++) {
            printf("%02x ", trame->preambule[i]);
        }
        printf("\nSFD: %02x\n", trame->sfd);
        printf("Destination MAC: ");
        for (int i = 0; i < 6; i++) {
            printMACAddress(trame->destination_mac);
        }
        printf("\nSource MAC: ");
        for (int i = 0; i < 6; i++) {
            printMACAddress(trame->source_mac);
        }
        printf("\nType: %04x\n", trame->type);
        printf("Data: ");
        
        printf("\nFCS: %08x\n", trame->fcs);
    } else {
        // Affichage en mode utilisateur
        printf("Preambule: (synchronisation)\n");
        printf("SFD: (début de l'info utile)\n");
        printf("Destination MAC: ");        
            printMACAddress(trame->destination_mac);
        
        printf("\nSource MAC: ");
        printMACAddress(trame->source_mac);
        printf("\nType: ");
        switch (trame->type) {
            case 0x0800:
                printf("IPv4");
                break;
            case 0x0806:
                printf("ARP");
                break;
            case 0x86DD:
                printf("IPv6");
                break;
            default:
                printf("%04x (inconnu)", trame->type);
                break;
        }
        printf("\nData: (données)\n");
        //print données ?
        printf("FCS: (code polynomial détecteur d'erreurs)\n");
    }
}

/// Partie 4
bool stp(reseau *r)
{
    /*un switch a nbports : 
        switch 1: 
            ports[nbports]
            etat_ports[nbports]
            port[1] --> etat_ports[1]            
    */
   /*
    // election switch racine
    for (size_t i = 0; i < r->nb_equipements; i++)
    {
        
    }
    */
   return false;
}


int main() {
    
    PARTIE 2    
    // Créer une structure de réseau vide
    reseau r;

    // Lire le fichier de configuration et créer le réseau
    lire_config("reseau_config.txt", &r);

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
            printMACAddress(r.m_equipements[i].data.m_switch.mac);
            printf("  Nombre de ports : %d\n", r.m_equipements[i].data.m_switch.nbport);
            printf("  Priorité : %d\n", r.m_equipements[i].data.m_switch.priorite);
            printf("  Table de commutation :\n");
            printSwitchTable(&r.m_equipements[i].data.m_switch);
        }
    }

    // Libérer la mémoire allouée pour les équipements et la structure de graphe
    free(r.m_equipements);
    free_graphe(&r.m_graphe);
    
    return 0;

    /*
   // Partie 3
   // Créer une structure de réseau vide
    reseau r;

    // Lire le fichier de configuration et créer le réseau
    lire_config("reseau_config.txt", &r);

     // Création d'une trame Ethernet pour un échange entre st1 et st2
     trame_ethernet trame;
    for(int i = 0; i < 7; i++) {
        trame.preambule[i] = 0xaa;
    }
    trame.preambule[6] = 0xab;
    trame.sfd = 0xab;
    for(int i = 0; i < 6; i++) {
        trame.destination_mac.octets[i] = mac_st2.octets[i];
        trame.source_mac.octets[i] = mac_st1.octets[i];
    }
    trame.type = 0x0800; // type IPv4
    memcpy(trame.data, "Hello, world!", 13); // Données de la trame : "Hello, world!"
    trame.fcs = 0; // FCS de la trame : à calculer ultérieurement

    // Envoi de la trame Ethernet de st1 à sw1
    printf("Envoi de la trame Ethernet de st1 à sw1 :\n");
    afficher_trame_ethernet(trame);
    recevoirTrame(&sw1, &trame, 1);

    // Réémission de la trame Ethernet de sw1 à st2
    printf("\nRéémission de la trame Ethernet de sw1 à st2 :\n");
    afficher_trame_ethernet(trame);
    recevoirTrame(&st2, &trame, 2);

    // Affichage de la réception de la trame Ethernet par st2
    printf("\nst2 a reçu la trame Ethernet de st1 avec les données suivantes : ");
    for (int i = 0; i < 13; i++) {
        printf("%c", trame.data[i]);
    }
    printf("\n");
*/
    return 0;


}
