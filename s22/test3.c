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
    MACAddress m_macAddress; //NOTE TODO On pourrai Changer en Tab de MAC
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
    TableDeCommutation table; //c dedans qu'il y a ces ports
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

/////// Switch fonctions
// pour crer un switch 
void initSwitch(Switch *sw, MACAddress MAC, int nbport, int priorite) {
    sw->mac = MAC;
    sw->nbport = nbport;
    sw->priorite = priorite;

    // Initialiser la table de commutation
    sw->table.capacite = nbport;
    sw->table.nb_entrees = 0;
    sw->table.entrees = malloc(nbport * sizeof(switchTableEntry));
    if (sw->table.entrees == NULL) {
        // Gérer l'erreur d'allocation mémoire
        exit(EXIT_FAILURE);
    }
    // Initialiser les ports à -1 (= vide -> servira pour FirstFreePort)
    for (int i = 0; i < nbport; i++) {
        sw->table.entrees[i].m_port = -1;
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
    sw->table.entrees[sw->table.nb_entrees].m_port = port; //en soit ya pas de port donné, se sera le premier port 'libre' qui sera enregistré
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
    printf("%02x:%02x:%02x:%02x:%02x:%02x", mac.octets[0], mac.octets[1], mac.octets[2],
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

//renvoi mac d'un equipement
MACAddress getMAC(equipement eqt)
{
    if (eqt.type == STATION)
    {
        return eqt.data.m_station.m_adresseMac;
    }
    else 
    {
        return eqt.data.m_switch.mac;
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


// Fonction pour afficher une trame Ethernet
void printTrame(trame_ethernet *trame) {
    printf("Trame Ethernet:\n");
    
    printf("Preambule: ");
    for (int i = 0; i < 7; i++) {
        printf("%02x ", trame->preambule[i]);
    }
    printf("\nSFD: %02x\n", trame->sfd);
    printf("MAC Source: ");
    printMACAddress(trame->source_mac);
    printf("\nMAC Destination: ");
    printMACAddress(trame->destination_mac);
    printf("\nType: %04x\n", trame->type);
    printf("Data: %s\n", trame->data);
    printf("FCS: %08x\n", trame->fcs);
    printf("\n", trame->fcs);
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
            initSwitch(&e.data.m_switch, mac, nbport, priorite);
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


//////////////// PARTIE 3 ////////////////

// initialise une trame
void init_trame(trame_ethernet *trame , MACAddress MAC_src, MACAddress MAC_dest , const char *message)
{
    /*  
    Le champ preambule est initialisé avec la valeur 0x55, qui est la valeur binaire 01010101. 
    Le champ sfd est initialisé avec la valeur 0xD5, qui est la valeur hexadécimale 11010101. 
    Le champ type est initialisé avec la valeur 0x0800, qui est le type IP par défaut. 
    Le champ data est initialisé avec le message spécifié, en utilisant la fonction strncpy pour copier les caractères du message dans le tableau data. 
    Le champ fcs est initialisé avec la valeur 0, car on s'en fou du fcs
    */

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




bool compare_mac(MACAddress mac1, MACAddress mac2) 
{
    return memcmp(mac1.octets, mac2.octets, sizeof(mac1.octets)) == 0;
}

//retourne pointeur vers equipement où MAC == mac 
equipement* get_equipement_by_mac(reseau *r, MACAddress mac) 
{
    printf("get_equipement_by_mac");
    for (size_t i = 0; i < r->nb_equipements; i++) 
    {
        if (r->m_equipements[i].type == STATION && compare_mac(r->m_equipements[i].data.m_station.m_adresseMac, mac)) 
        {
            return &r->m_equipements[i];
        } 
        else if (r->m_equipements[i].type == SWITCH && compare_mac(r->m_equipements[i].data.m_switch.mac, mac)) 
        {
            return &r->m_equipements[i];
        }       
    }
    return NULL;
}


int eqt_index(reseau *r , MACAddress mac)
{
    for (size_t i = 0; i < r->nb_equipements; i++) 
    {
        if (r->m_equipements[i].type == STATION &&
            compare_mac(r->m_equipements[i].data.m_station.m_adresseMac, mac)) 
        {
            return i;
        } 

        else if (r->m_equipements[i].type == SWITCH &&
                compare_mac(r->m_equipements[i].data.m_switch.mac, mac)) 
        {
            return i;
        }       
    }
    return -1;
}

//retourn MAC de equipement Connécté au port PORT de SW
MACAddress eqt_connecte_port(reseau *r,Switch *sw , int port)
{
    int indexSW = eqt_index(r,sw->mac);
    sommet sommets_adjs[ordre(&r->m_graphe)];
    int nb_adj = sommets_adjacents(&r->m_graphe, indexSW , sommets_adjs);

    return getMAC(r->m_equipements[sommets_adjs[port]]);
   
}

int MAC_In_SwitchTable(Switch *sw, MACAddress mac) {
    for (int i = 0; i < sw->table.nb_entrees; i++) {
        if (compare_mac(mac, sw->table.entrees[i].m_macAddress)) {
            return sw->table.entrees[i].m_port;
        }
    }
    return -1;
}

void recevoir_Trame(trame_ethernet *trame , MACAddress MACdest)
{
    if (compare_mac(trame->destination_mac ,MACdest))
    {
        printMACAddress(trame->destination_mac);printf("tram_MACdest == currentMAC ");printMACAddress(MACdest) ;printf(" trame reçus YOUPI\n");
    }
    else
    {
        printf("je suis : ");printMACAddress(MACdest);printf("trame Pas pour moi\n");
    }
    
}

bool transferer_trame(reseau *r, MACAddress *current, MACAddress *destination, trame_ethernet *trame) {

    // Print situation
    //print(MAC_current --> MAC_destination)
    printMACAddress(*(current));
    printf(" --> ");
    printMACAddress(*(destination));
    printf("manger");

    // regarde si dest == STATION
    // Si l'équipement est la destination
    //equipement* eqt_dest = get_equipement_by_mac(r,*destination);
    equipement* eqt_dest = get_equipement_by_mac(r,*destination);
    if (&eqt_dest->type == STATION) //erreur segmentation
    {
        printf("destination == station");
        recevoir_Trame(trame,*destination);
    }
    else //destination.type == SWITCH
    {        
        printf("destination == SWITCH");
        Switch *sw = (Switch*)eqt_dest;
        //sinon , regarder dans table de commutation        
        int port = MAC_In_SwitchTable(sw, trame->destination_mac);
        if (port != -1) //if trame_MACdest est dans tableCOmmut
        {
            MACAddress newMAC = eqt_connecte_port(r,sw ,port );
            transferer_trame(r,destination,&newMAC,trame);
        }
        else
        {
            // Sinon, regarder sommets adj    
            sommet sommets_adjs[ordre(&r->m_graphe)];
            int nb_adj = sommets_adjacents(&r->m_graphe, get_equipement_by_mac(r,*destination) - r->m_equipements, sommets_adjs);
            for (int i = 0; i < nb_adj; i++) // foreach equipement_sommet_adj
            {
                equipement *adj_equip = &r->m_equipements[sommets_adjs[i]];    
                MACAddress adj_MAC =  getMAC(*adj_equip);
                if (current == &adj_MAC)
                {
                    printf("C'est le sommet duquel j'ai reçus Trame , PASS \n");
                    //est ce que ce ne serait pas là que j'ajouterais dans TableCommut ? oui mais si ya 2 switch dans reseau , commnet second switch enregistre numport pour que st1 , st2,st3 sont tous sur port 1 ?
                }
                else /*(adj_equip != current) */
                {               
                    transferer_trame(r, destination, &adj_MAC, trame);                
                }
            }        
        }
    }
    printf("après avoir vu tt les sommets , tjr pas réussi. ya pteter un prblm\n");
    return false; // Aucun transfert réussi
}


void envoyer_trame(reseau *r, trame_ethernet *trame) 
{
    // Trouver le sommet correspondant à l'adresse MAC source
        int numSource = eqt_index(r,trame->source_mac);
        printf("envoyer trame");
        // Envoyer la trame au sommet adjacent
        sommet sommets_adjs[ordre(&r->m_graphe)];
        int nb_adj = sommets_adjacents(&r->m_graphe, numSource, sommets_adjs);                    
        MACAddress mac = getMAC(r->m_equipements[sommets_adjs[0]]);  //dans tt les cas 1 station relié slmt à 1 switch          
        transferer_trame(r, &trame->source_mac , &mac, trame);
}
            
            
        


// Partie 4
void stp(reseau *r)
{

}


int main() {
    
    reseau r;
    init_reseau(&r);
    lire_config("reseau_config.txt", &r);
    printReseau(&r);
    equipement sw0 = r.m_equipements[0];
    equipement st1 = r.m_equipements[1];    
    equipement st2 = r.m_equipements[2];
    equipement st3 = r.m_equipements[3];


    trame_ethernet trame;
    init_trame(&trame, r.m_equipements[1].data.m_station.m_adresseMac,r.m_equipements[2].data.m_station.m_adresseMac, "Hello, Station 2 from Station 1!");
    printTrame(&trame);

    // Envoyer une trame de la station 1 à la station 2
    
    MACAddress mac1 = getMAC(st1);
    printf("mac1 : ");printMACAddress(mac1);printf("\n");
    MACAddress mac2 = getMAC(st2);
    printf("mac2 : ");printMACAddress(mac2);printf("\n");
    printf("test");
    envoyer_trame(&r,&trame);


    return 0;    
}
