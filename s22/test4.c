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

typedef struct MACAddress {
    uint8_t octets[6];
} MACAddress;


typedef enum PortState{
    PORT_BLOCKED,
    PORT_ROOT,
    PORT_DESIGNATED
} PortState;

// Structure pour les BPDUs
typedef struct {
    MACAddress root_id;        // Identifiant de la racine
    uint32_t cost;             // Coût du chemin vers la racine
    MACAddress transmitting_id; // Identifiant du pont émetteur
} BPDU;

typedef struct switchTableEntry {
    MACAddress *tab_macAddresses;  // Tableau dynamique d'adresses MAC
    int nb_mac;                // Nombre d'adresses MAC sur ce port
    int capacite;              // Capacité du tableau d'adresses MAC
    int m_port;
    PortState port_state; // Ajout de l'état du port
    BPDU bpdu;           // BPDU sur le port de la switchtableEntry
} switchTableEntry;

typedef struct TableDeCommutation {
    int capacite;         // Capacité maximale des entrées
    int nb_entrees;       // Nombre d'entrées actuelles
    switchTableEntry *tab_entrees;
} TableDeCommutation;



// switch
typedef struct Switch {
    MACAddress mac;
    int nbport;
    int priorite;
    TableDeCommutation table;
    PortState *port_states; // Tableau des états des ports
} Switch; 

// Structure de la station
typedef struct station{
    IPAddress m_adresseIP;
    MACAddress m_adresseMac;
}station;

//equipement 
typedef struct equipement {
    enum type_equipement { STATION, SWITCH } type;
    union {
        station m_station;
        Switch m_switch;
    } data;
} equipement;

typedef struct reseau {
    graphe m_graphe;
    equipement* tab_equipements;
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

// fonctions PORTS #####################""
// (STP) définit l'état d'un port dans une (entrée du) switch 
void setPortState(Switch *sw, int port, PortState state) {
    if (port < 0 || port >= sw->nbport) {
        printf("Erreur : port invalide\n");
        return;
    }
    sw->port_states[port] = state;
}

//fonction initBPDU avec rootMAC , cout , MAC du switch qui envoi la bpdu
void initBPDU(BPDU *bpdu, MACAddress root_id, uint32_t cost, MACAddress transmitting_id) {
    bpdu->root_id = root_id;
    bpdu->cost = cost;
    bpdu->transmitting_id = transmitting_id;
}

/////////////////////

//Fonctions SWITCH      #####################""

// FONCTIONS PRINTS 


// Fonction pour afficher une adresse IP
void printIPAddress(IPAddress ip) {
    printf("%d.%d.%d.%d", ip.octet[0], ip.octet[1],ip.octet[2], ip.octet[3]);
}


// Fonction pour afficher une adresse Mac
void printMACAddress(MACAddress mac) {
    printf("%02x:%02x:%02x:%02x:%02x:%02x",
           mac.octets[0], mac.octets[1], mac.octets[2],
           mac.octets[3], mac.octets[4], mac.octets[5]);

}


// Fonction pour afficher une entrée de la table de commutation d'un switch
void printSwitchTableEntry(switchTableEntry entry) {
    printf("Port : %d\n", entry.m_port);
    printf("nb_MACs : %d\n", entry.nb_mac);
    printf("port_state : ");
    switch(entry.port_state) {
        case PORT_BLOCKED:
            printf("Bloqué\n");
            break;
        case PORT_ROOT:
            printf("Root\n");
            break;
        case PORT_DESIGNATED:
            printf("Désigné\n");
            break;
        default:
            printf("Inconnu\n");
    }
    printf("tab_tab_MACaddresses :\n");
    for(int i = 0; i < entry.nb_mac; i++) 
    {
        printf("%d : ",i);
        printMACAddress(entry.tab_macAddresses[i]);
        printf("\n");
    }
    printf("\n");
}

// Fonction pour afficher une table de commutation d'un switch
void printSwitchTable(TableDeCommutation table) {
    printf("Table de commutation :\n");
    for(int i = 0; i < table.nb_entrees; i++) {
        printf("Entrée %d :\n", i+1);
        printSwitchTableEntry(table.tab_entrees[i]);
    }
}

// Fonction pour afficher les informations d'un switch
void printSwitch(Switch sw) {
    printf("Switch :\n");
    printf("— adresse MAC : ");
    printMACAddress(sw.mac);
    printf("\n— nb_ports : %d\n", sw.nbport);
    printf("— priorité : %d\n", sw.priorite);
    printSwitchTable(sw.table);
    printf("\n");
}

// Fonction pour afficher les informations d'une station
void printStation(station s) {
    printf("Station :\n");
    printf("— adresse IP : ");
    printIPAddress(s.m_adresseIP);
    printf("\n— adresse MAC : ");
    printMACAddress(s.m_adresseMac);
    printf("\n\n");
}

// Fonction pour afficher les informations d'un équipement
void printEquipement(equipement equip) {
    if(equip.type == STATION) {
        printf("Type d'équipement : Station\n");
        printStation(equip.data.m_station);
    } else if(equip.type == SWITCH) {
        printf("Type d'équipement : Switch\n");
        printSwitch(equip.data.m_switch);
    }
}

// Fonction pour afficher le réseau
// Fonction pour afficher les informations d'un réseau
void printReseau(reseau r) {
    printf("Reseau :\n");
    // Affichage des équipements
    for (size_t i = 0; i < r.nb_equipements; i++) {
        printf("%zu. ", i);
        if (r.tab_equipements[i].type == SWITCH) {
            printSwitch(r.tab_equipements[i].data.m_switch);
        } else if (r.tab_equipements[i].type == STATION) {
            printStation(r.tab_equipements[i].data.m_station);
        }
    }
    // Affichage du graphe du réseau
    printf("Affichage du graphe :\n");
    afficher(&r.m_graphe);
}





// init la tableCommut avec la capacité : 0 mac , capcité d'entree capacite (fo donner 3 par défaut)
void initTableDeCommutation(TableDeCommutation *table, int capacite, MACAddress MACswitch) {
    table->capacite = capacite;
    table->nb_entrees = 0; 
    table->tab_entrees = malloc(capacite * sizeof(switchTableEntry)); //tab d'entrées
    for (int i = 0; i < capacite; i++) 
    {   //foreach switchTableEntry --> init entry
        table->tab_entrees[i].nb_mac = 0; 
        table->tab_entrees[i].capacite = 1;
        table->tab_entrees[i].m_port = -1; // port associé , -1 == aucun
        table->tab_entrees[i].tab_macAddresses = malloc(1 * sizeof(MACAddress)); //tab_MAC => 1 slot
        table->tab_entrees[i].port_state = PORT_BLOCKED;
        initBPDU(&table->tab_entrees[i].bpdu, MACswitch, 0, MACswitch);
    }
}

//dans une TableCommut , renvoi le num_port où se trouve MAC
int trouverPortPourMAC(TableDeCommutation *table, MACAddress mac) {
    for (int i = 0; i < table->nb_entrees; i++) {
        for (int j = 0; j < table->tab_entrees[i].nb_mac; j++) {
            if (memcmp(&table->tab_entrees[i].tab_macAddresses[j], &mac, sizeof(MACAddress)) == 0) {
                return table->tab_entrees[i].m_port;
            }
        }
    }
    return -1; // Adresse MAC non trouvée
}
  
// renvoi une Entry asociée à un port s'il la trouve dans TableCOmmut. Sinon renvoi NULL
switchTableEntry* rechercherEntreeParPort(TableDeCommutation *table, int port) {
    for (int i = 0; i < table->nb_entrees; i++) {
        if (table->tab_entrees[i].m_port == port) {
            return &table->tab_entrees[i];
        }
    }
    return NULL;
}

// add mac to tab_MAC d'une Entry
void ajouterMacAUneEntree(switchTableEntry *entree, MACAddress mac) {

    if (entree->nb_mac == entree->capacite) {
        //printf("ajouter MAC à une entree : \n");
        // Redimensionner le tableau dynamique si nécessaire
        entree->capacite =  entree->capacite * 2;
        entree->tab_macAddresses = realloc(entree->tab_macAddresses, entree->capacite * sizeof(MACAddress));
    }
    //printf("ajouter MAC à une entree. PRINT DE LA NOUVELLE ENTRY \n");
    entree->tab_macAddresses[entree->nb_mac] = mac; // place le nvMAC en ernière position
    entree->nb_mac++; // on ajout 1 au nb _mac
    //printSwitchTableEntry(*entree);
}

// add Entry à une TabCommut 
// add Entry à une TabCommut
void ajouterEntree(TableDeCommutation *table, MACAddress mac, int port, MACAddress root_id, uint32_t cost, MACAddress transmitting_id) {
    switchTableEntry *entree = rechercherEntreeParPort(table, port); //checker si ya déjà une entrée associé au port pcq on va addMAC à cet endroit
    if (entree == NULL) {
        //if port pas d'entree associé, ajout de la nv entree avec le port
        if (table->nb_entrees == table->capacite) {
            // Redimensionner le tableau dynamique si nécessaire
            table->capacite *= 2;
            table->tab_entrees = realloc(table->tab_entrees, table->capacite * sizeof(switchTableEntry));
        }
        entree = &table->tab_entrees[table->nb_entrees]; // newEntry => dernière position dans table.tab_entrees
        entree->m_port = port;
        entree->nb_mac = 0;
        entree->capacite = 1;
        entree->tab_macAddresses = malloc(1 * sizeof(MACAddress)); //tab_MAC => 1 slot
        table->nb_entrees++;
    }
    initBPDU(&entree->bpdu, root_id, cost, transmitting_id);
    ajouterMacAUneEntree(entree, mac);
}

// pour crer un switch 
void initSwitch(Switch *sw, MACAddress mac, int nbport, int priorite) {
    sw->mac = mac;
    sw->nbport = nbport;
    sw->priorite = priorite;    

    //creation de TableCOmmut
    initTableDeCommutation(&sw->table ,nbport,mac);

    //(STP) init des ports 
    sw->port_states = malloc(nbport * sizeof(PortState));
    for (int i = 0; i < nbport; i++) {
        sw->port_states[i] = PORT_BLOCKED; // Initialiser tous les ports à l'état BLOQUED => inactif (qd pas STP) par défaut             
        // NOTE : quand stp , foreach port in port_states --> if STATE != BLOCKED , transfer_trame. Else , rien.
    }
}


//////////////////////////////////////////

// fonctions autres

// Fonction pour récupérer l'adresse MAC d'un equipement donné
MACAddress* get_mac_address(equipement *equipment) {
    if (equipment->type == STATION) {
        return &equipment->data.m_station.m_adresseMac;
    } else if (equipment->type == SWITCH) {
        return &equipment->data.m_switch.mac;
    }
    return NULL; // Si le type est inconnu
}





void init_reseau(reseau *r) {
    // Initialiser le graphe
    init_graphe(&r->m_graphe);

    // Initialiser le tableau d'équipements
    r->tab_equipements = NULL;
    r->nb_equipements = 0;
}



void ajouter_equipement(reseau *r, equipement equip) {
    r->nb_equipements++;
    r->tab_equipements = realloc(r->tab_equipements, r->nb_equipements * sizeof(equipement));
    r->tab_equipements[r->nb_equipements - 1] = equip;
    ajouter_sommet(&r->m_graphe);
}


// FREE ////////////////////////////////

void freeSwitch(Switch *s) {
    free(s->table.tab_entrees);
}

void freeReseau(reseau *r) {
    for (size_t i = 0; i < r->nb_equipements; i++) {
        if (r->tab_equipements[i].type == SWITCH) {
            freeSwitch(&r->tab_equipements[i].data.m_switch);
        }
    }
    free(r->tab_equipements);
    free_graphe(&r->m_graphe);
}


/////////////// 




////////////////////////

// PARTIE 2
// Lecture d'une adresse MAC depuis une chaîne de caractères
void read_mac_address(const char *str, MACAddress *mac) {
    sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &mac->octets[0], &mac->octets[1], &mac->octets[2],
           &mac->octets[3], &mac->octets[4], &mac->octets[5]);
}

// Lecture d'une adresse IP depuis une chaîne de caractères
void read_ip_address(const char *str, IPAddress *ip) {
    sscanf(str, "%hhu.%hhu.%hhu.%hhu",
           &ip->octet[0], &ip->octet[1], &ip->octet[2], &ip->octet[3]);
}



void lireMAC(char *str, MACAddress *mac) {
    sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &mac->octets[0], &mac->octets[1], &mac->octets[2],
           &mac->octets[3], &mac->octets[4], &mac->octets[5]);
}

void lireIP(char *str, IPAddress *ip) {
    sscanf(str, "%hhu.%hhu.%hhu.%hhu",
           &ip->octet[0], &ip->octet[1], &ip->octet[2], &ip->octet[3]);
}

// Fonction pour lire la configuration depuis un fichier
void lire_config(const char *filename, reseau *r) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    // Initialiser le réseau et le graphe
    init_reseau(r);

    // Lire le nombre d'équipements et de liens
    int nb_equipements, nb_liens;
    fscanf(file, "%d %d", &nb_equipements, &nb_liens);

    // Lire les équipements
    for (int i = 0; i < nb_equipements; i++) {
        int type;
        fscanf(file, "%d;", &type);

        equipement equip;
        if (type == 1) {  // Station
            equip.type = STATION;
            char mac_str[18], ip_str[16];
            fscanf(file, "%[^;];%s", mac_str, ip_str);

            lireMAC(mac_str, &equip.data.m_station.m_adresseMac);
            lireIP(ip_str, &equip.data.m_station.m_adresseIP);
        } else if (type == 2) {  // Switch
            equip.type = SWITCH;
            char mac_str[18];
            int nbport, priorite;
            fscanf(file, "%[^;];%d;%d", mac_str, &nbport, &priorite);

            MACAddress mac;
            lireMAC(mac_str, &mac);
            initSwitch(&equip.data.m_switch, mac, nbport, priorite);
        }

        ajouter_equipement(r, equip);
    }

    // Lire les liens
    for (int i = 0; i < nb_liens; i++) {
        int e1, e2, poids;
        fscanf(file, "%d;%d;%d", &e1, &e2, &poids);

        arete a;
        a.s1 = e1;
        a.s2 = e2;
        a.poids = poids;
        ajouter_arete(&r->m_graphe, a);
    }

    fclose(file);
}



// PARTIE 3     #####################""

void init_trame(trame_ethernet *trame , MACAddress MAC_src, MACAddress MAC_dest , const char *message)
{
    // Initialiser les champs de la trame
    memset(trame->preambule, 0x55, sizeof(trame->preambule));
    trame->sfd = 0xD5;
    trame->destination_mac = MAC_dest;
    trame->source_mac = MAC_src;
    trame->type = 0x0800; // Type IP par défaut
    memset(trame->data, 0, sizeof(trame->data));
    strncpy((char *)trame->data, message, sizeof(trame->data));
    trame->fcs = 0; // Calcul du FCS non implémenté
}

//// fonction PRINT_trame
// Fonction pour afficher une trame Ethernet en mode "utilisateur"
void print_trame_ethernet_user(trame_ethernet trame) {
    printf("Destination MAC: ");
    printMACAddress(trame.destination_mac);
    printf("\n");

    printf("Source MAC: ");
    printMACAddress(trame.source_mac);
    printf("\n");

    printf("Type: %04x\n", trame.type);

    printf("Data: %s\n", trame.data);
}

// Fonction pour afficher une trame Ethernet en mode hexadécimal
void print_trame_ethernet_hex(trame_ethernet trame) {
    printf("Preambule: ");
    for (int i = 0; i < sizeof(trame.preambule); i++) {
        printf("%02x ", trame.preambule[i]);
    }
    printf("\n");

    printf("SFD: %02x\n", trame.sfd);

    printf("Destination MAC: ");
    printMACAddress(trame.destination_mac);
    printf("\n");

    printf("Source MAC: ");
    printMACAddress(trame.source_mac);
    printf("\n");

    printf("Type: %04x\n", trame.type);

    printf("Data: ");
    for (int i = 0; i < sizeof(trame.data); i++) {
        printf("%02x ", trame.data[i]);
    }
    printf("\n");

    printf("FCS: %08x\n", trame.fcs);
}



bool compare_mac(MACAddress mac1, MACAddress mac2) {
    return memcmp(mac1.octets, mac2.octets, sizeof(mac1.octets)) == 0;
}

equipement *get_equipement_by_mac(reseau *r, MACAddress mac) {
    for (size_t i = 0; i < r->nb_equipements; i++) {
        if (r->tab_equipements[i].type == STATION &&
            compare_mac(r->tab_equipements[i].data.m_station.m_adresseMac, mac)) {
            return &r->tab_equipements[i];
        } else if (r->tab_equipements[i].type == SWITCH &&
                   compare_mac(r->tab_equipements[i].data.m_switch.mac, mac)) {
            return &r->tab_equipements[i];
        }
    }
    return NULL;
}

//envoi trame 


void printTransferTrame(equipement src , equipement dest)
{
    if (src.type == SWITCH)
    {
        printMACAddress(src.data.m_switch.mac);;
    }
    else
    {
        printMACAddress(src.data.m_station.m_adresseMac);
    }
    printf(" --> ");
    if (dest.type == SWITCH)
    {
        printMACAddress(dest.data.m_switch.mac);
    }
    else
    {
        printMACAddress(dest.data.m_station.m_adresseMac);
    }
    printf("\n");
    
}
// relayage de trame 
void transfer_trame(reseau *r, equipement *source, equipement *destinataire, trame_ethernet *trame) {
    printTransferTrame(*source , *destinataire);

    // int index_source = source - r->tab_equipements;  // Calculer l'indice de la source dans le tableau
    //int index_destinataire = destinataire - r->tab_equipements;  // Calculer l'indice du destinataire dans le tableau

    //printf("Index source: %d\n", index_source);  // Pour debug
    //printf("Index destinataire: %d\n", index_destinataire);  // Pour debug

    if (destinataire->type == SWITCH) {
        Switch *sw = &destinataire->data.m_switch;
        
        // Récupérer les sommets adjacents du switch destinataire
        sommet sa[r->m_graphe.ordre]; // Taille à ajuster selon les besoins
        size_t nb_adj = sommets_adjacents(&r->m_graphe, destinataire - r->tab_equipements, sa);
        
        //printf("nb_sommet adjacent : %d \n", nb_adj);

        // Trouver le port source
        int port_src = -1;
        for (size_t i = 0; i < nb_adj; i++) {
            if (sa[i] == (source - r->tab_equipements)) {
                //printf("port_src : %d\n",port_src);
                port_src = i;
                break;
            }
            //printf("num_Sommet_adj : %d // comparé à : ",sa[i],(source - r->tab_equipements));
            //printf("port_src  %d\n",port_src);
        }

        // Vérifier si le MAC source est dans la table de commutation. If(non) , ajouter
        if (trouverPortPourMAC(&sw->table, trame->source_mac) == -1) {
            ajouterEntree(&sw->table, trame->source_mac, port_src, sw->mac, 0, sw->mac);        
        }
        printf("port_src trv : %d\n",port_src);


        // Vérifier si le MAC_dest est dans la table de commutation. If(oui , envoyer à ce port) , sinon , broadcast
        // trouver le port_dest
        int port_dest = -1;  
        /*      
        for (size_t i = 0; i < nb_adj; i++) {
            equipement *adj_eq = &r->tab_equipements[sa[i]]; //adj_eqt => l'equipement "i" dans le tab_eqt_adj
            if (adj_eq->type == STATION) { //foreach eqt_adj
                station *sta = &adj_eq->data.m_station;
                if (memcmp(&sta->m_adresseMac, &trame->destination_mac, sizeof(MACAddress)) == 0) { //if 
                    port_dest = i;
                    break;
                }
            }
        } */

        port_dest = trouverPortPourMAC(&sw->table, trame->destination_mac);                
        if (port_dest != -1) {          

            // if  port_dest trouvé , Transférer la trame via le port correspondant
            transfer_trame(r, destinataire, &r->tab_equipements[sa[port_dest]], trame);
        } else {
            // sinon , transfer la trame à tous les sommets adjacents sauf celui de la source
            for (size_t i = 0; i < nb_adj; i++) {
                if (sa[i] != (source - r->tab_equipements)) {
                    transfer_trame(r, destinataire, &r->tab_equipements[sa[i]], trame);
                }
            }
        }
    } 
    else if (destinataire->type == STATION) 
    {
        printf("dest.type : station : \n");
        station *sta = &destinataire->data.m_station;
        if (memcmp(&sta->m_adresseMac, &trame->destination_mac, sizeof(MACAddress)) == 0) 
        {
            printf("Je suis MAC: ");
            printMACAddress(sta->m_adresseMac);
            printf(", j'ai reçu ma trame. Disant : \n");
            printf("< %s >\n", trame->data);
        } 
        else {
            printf("Je suis MAC: ");
            printMACAddress(sta->m_adresseMac);
            printf(", trame pas pour moi.\n");
        }
    }
}

// stationsource envoi une trame dans le reseau 
void envoi_trame(reseau *r, equipement *stationsource, trame_ethernet *trame) {
    // Obtenir les sommets adjacents de la station source
    sommet sa[r->m_graphe.ordre]; // Taille à ajuster selon les besoins
    size_t nb_adj = sommets_adjacents(&r->m_graphe, stationsource - r->tab_equipements, sa);
    //printf("nb_sommet adjacent : %u \n", nb_adj);
    // Assumer qu'une station n'a qu'un seul sommet adjacent (un switch)    
    transfer_trame(r, stationsource, &r->tab_equipements[sa[0]], trame);
    
}

/// Partie 4



// demander si c'est 1 BPDU  pour 2 port

int main() {
    reseau r;
    // Initialiser le réseau à partir du fichier de configuration
    lire_config("reseau_config.txt", &r);
    printReseau(r);

    equipement *sw0 = &r.tab_equipements[0];
    if (sw0->type == SWITCH) {
        printf("sw0 : SWITCH\n");
    } else {
        printf("sw0 : STATION\n");
    }

    equipement *st1 = &r.tab_equipements[1];
    equipement *st2 = &r.tab_equipements[2];

    // Initialiser la trame
    trame_ethernet trame;
    init_trame(&trame, *get_mac_address(st1), *get_mac_address(st2), "message de st1 à st2");

    // Envoyer la trame
    envoi_trame(&r, st1, &trame);

    // Afficher la table de commutation après l'envoi de la trame
    printSwitch(sw0->data.m_switch);
    // Free mémoire
    freeReseau(&r);
    return 0;
}
