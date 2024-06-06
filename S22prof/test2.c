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


// PARTIE 3

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




bool compare_mac(MACAddress mac1, MACAddress mac2) {
    return memcmp(mac1.octets, mac2.octets, sizeof(mac1.octets)) == 0;
}
equipement *get_equipement_by_mac(reseau *r, MACAddress mac) {
    for (size_t i = 0; i < r->nb_equipements; i++) {
        if (r->m_equipements[i].type == STATION &&
            compare_mac(r->m_equipements[i].data.m_station.m_adresseMac, mac)) {
            return &r->m_equipements[i];
        } else if (r->m_equipements[i].type == SWITCH &&
                   compare_mac(r->m_equipements[i].data.m_switch.mac, mac)) {
            return &r->m_equipements[i];
        }
    }
    return NULL;
}


void transferer_trame(reseau *r, equipement *current, equipement *destination, trame_ethernet *trame) 
{
    if (destination->type == STATION) {
        equipement *dst = get_equipement_by_mac(r, destination->data.m_station.m_adresseMac); // j'utilise sa pour l'affichage
        if (compare_mac(destination->data.m_station.m_adresseMac, trame->destination_mac)) {
            equipement *src = get_equipement_by_mac(r, trame->source_mac);            
            printf("Je suis Station %ld du réseau, j'ai reçu le message de Equipement %ld : \"%s\"\n",
                   (dst - r->m_equipements) , (src - r->m_equipements), trame->data);  

            /*
            // Créer une nouvelle trame Ethernet
            trame_ethernet trameCONFIRMED;

            // Initialiser la trame avec les adresses MAC des équipements et le message
            init_trame(&trameCONFIRMED, destination->data.m_station.m_adresseMac, current->data.m_switch.mac, "J'ai reçu trame");

            // Simuler l'envoi de la trame en appelant la fonction transferer_trame
            transferer_trame(r, destination, current, &trameCONFIRMED);
            
            //envoyer_trame();
            */

        } else {
            printf("Je suis Equipement %ld, le message n'est pas pour moi\n", (dst - r->m_equipements));
        }
    } 
    else if (destination->type == SWITCH) 
    {
        Switch *sw = &destination->data.m_switch;
        

         //regarde si message déstiné à lui , possible que c pr confirmé un message reçu
         equipement *sw_dst = get_equipement_by_mac(r, destination->data.m_station.m_adresseMac); // j'utilise sa pour l'affichage
        if (compare_mac(sw->mac, trame->destination_mac)) {
            equipement *src = get_equipement_by_mac(r, trame->source_mac);            
            printf("Je suis Switch (eqt %ld) du réseau, j'ai reçu le message de Equipement %ld : \"%s\"\n",
                   (sw_dst- r->m_equipements) , (src - r->m_equipements), trame->data);        
        } 
        else 
        {
            // c pas déstiné , à lui
            printf("Je suis Switch %ld, le message n'est pas pour moi ATTENTION SI BOUCLE\n", (sw_dst - r->m_equipements));            

            // if MAC_src pas dans tabCommut , MAJ tabCommut du switch actuel avec l'adresse MAC source        
            bool commutDest_found = false;
            for (int k = 0; k < sw->table.nb_entrees; k++) 
            {
                if (compare_mac(sw->table.entrees[k].m_macAddress, trame->destination_mac)) 
                {
                    //si c vu dans la tableCOmmut --> ne pas ajouter 
                    commutDest_found = true;                               
                    break;
                }
                if (!commutDest_found)
                {       //si c pas dejà dans la tableCOmmut --> ajouter tram.source_mac 
                    addSwitchTableEntry(sw, trame->source_mac, current - r->m_equipements);
                }
                
            }
            
        

            bool dest_found = false;
            // regarde dans CommutTable si Mac_Dest est connue
            for (int k = 0; k < sw->table.nb_entrees; k++) {
                if (compare_mac(sw->table.entrees[k].m_macAddress, trame->destination_mac)) {
                    //si c vu dans la tableCOmmut --> transfert vers le MAC connue
                    dest_found = true;
                    equipement *dest = get_equipement_by_mac(r, sw->table.entrees[k].m_macAddress);
                    transferer_trame(r, destination, dest, trame);
                    break;
                }
            }

            //si finalement c'est pas connu dans la table de commut
            if (!dest_found) {
                // BROADCAST (sauf vers sommet source)
                sommet *sommets_adjs = malloc(sw->nbport * sizeof(sommet)); //nbport*sommet --> c pas un prlbm nn ?
                int nb_adj = sommets_adjacents(&r->m_graphe, destination - r->m_equipements, sommets_adjs);

                for (size_t i = 0; i < nb_adj; i++) {
                    equipement *new_dest = &r->m_equipements[sommets_adjs[i]];
                    if (new_dest != current) {
                        transferer_trame(r, destination, new_dest, trame);
                    }
                }
                            
                free(sommets_adjs);
            }
        } 
    }
}

//Est ce que ce serait mieux d'avoir les MAC directmeent en paramètre ou les eqt
void envoyer_trame(reseau *r, equipement *eqt1, equipement *eqt2, const char *message) {
    // Créer une nouvelle trame Ethernet
    trame_ethernet trame;

    // Initialiser la trame avec les adresses MAC des équipements et le message
    init_trame(&trame, eqt1->data.m_station.m_adresseMac, eqt2->data.m_station.m_adresseMac, message);

    // Simuler l'envoi de la trame en appelant la fonction transferer_trame
    transferer_trame(r, eqt1, eqt2, &trame);
}







int main() {
    
    reseau r;
    init_reseau(&r);
    lire_config("reseau_config.txt", &r);
    printReseau(&r);
    equipement sw0 = r.m_equipements[0];
    if (sw0.type == STATION)
    {
        printf("sw0 : STATION\n");
    }
    else
    {
        printf("sw0 : SWITCH\n");
    }
    equipement st1 = r.m_equipements[1];
    if (st1.type == STATION)
    {
        printf("st1 : STATION\n");
    }
    else
    {
        printf("st1 : SWITCH\n");
    }
    
    equipement st2 = r.m_equipements[2];
    equipement st3 = r.m_equipements[3];


    // Envoyer une trame de la station 1 à la station 2
    envoyer_trame(&r, &st1, &st2, "Hello, st2 from st1!");

    // Envoyer une trame de la station 2 à la station 3
    envoyer_trame(&r, &st2, &st3, "Hello, st3 from st2!");
    // Libérer la mémoire allouée pour le réseau
    free(r.m_equipements);
    free_graphe(&r.m_graphe);

    return 0;
    
}
