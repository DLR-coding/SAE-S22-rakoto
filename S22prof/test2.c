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

typedef struct port
{
    int numport;
    int etat 
}port;


// switch
typedef struct Switch
{
    MACAddress mac;
    int nbport;
    port *tab_port;
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




bool compare_mac(MACAddress mac1, MACAddress mac2) 
{
    return memcmp(mac1.octets, mac2.octets, sizeof(mac1.octets)) == 0;
}
equipement *get_equipement_by_mac(reseau *r, MACAddress mac) 
{
    for (size_t i = 0; i < r->nb_equipements; i++) 
    {
        if (r->m_equipements[i].type == STATION &&
            compare_mac(r->m_equipements[i].data.m_station.m_adresseMac, mac)) 
        {
            return &r->m_equipements[i];
        } 
        else if (r->m_equipements[i].type == SWITCH &&
                compare_mac(r->m_equipements[i].data.m_switch.mac, mac)) 
        {
            return &r->m_equipements[i];
        }       
    }
    return NULL;
}

bool transferer_trame(reseau *r, equipement *current, equipement *destination, trame_ethernet *trame) {
    // Print situation
    if (current->type == STATION)
    {
        printf("Je suis STATION MAC: ");
        printMACAddress(current->data.m_station.m_adresseMac);
        if (destination->type == SWITCH)
        {            
            printf(" J'envoi trame à switch MAC: ");
            printMACAddress(destination->data.m_switch.mac);
        }
        else
        {            
            printf(" Station reçois trame d'une station --> bizzare");
        }
        printf(" disant \"%s\"\n", trame->data);
    }
    if (current->type == SWITCH)
    {
        printf("Je suis SWITCH MAC: ");
        printMACAddress(current->data.m_station.m_adresseMac);
        printf("J'envoi trame à  ");
        if (destination->type == SWITCH)
        {            
            printf("SWITCH MAC : ");
            printMACAddress(destination->data.m_switch.mac);
        }
        else
        {            
            printf(" switch reçois trame de Station : ");
            printMACAddress(destination->data.m_switch.mac);
        }
        printf(" disant \"%s\"\n", trame->data);
    }
    
    // regarde si Tram->MAC_Dest == Transfert_destination->MAC
    if ((compare_mac(destination->data.m_station.m_adresseMac, trame->destination_mac)) || (compare_mac(destination->data.m_switch.mac, trame->destination_mac))) 
    {
        // Si l'équipement est la destination
        if (destination->type == STATION)
        {
            printf("TRAM TRANSFÉRÉ ! \n Je suis station MAC: ");
            printMACAddress(destination->data.m_station.m_adresseMac);
            printf(", j'ai reçu le message venant de MAC: ");
            printMACAddress(trame->source_mac);
            printf(" disant \"%s\"\n", trame->data);
            printf("J'envoi un message au SWITCH en retour pour dire 'Je suis le desinataire , g bien reçus'\n");        
            // Créer une nouvelle trame Ethernet
            trame_ethernet trameCONFIRMED;

            // Initialiser la trame avec les adresses MAC des équipements et le message
            init_trame(&trameCONFIRMED, destination->data.m_station.m_adresseMac, current->data.m_switch.mac, "J'ai reçu trame");

            
            transferer_trame(r, destination, current, &trameCONFIRMED);                        
            
        }
        else // si c'est un switch (cas de Station dit au switch 'g reçus la trame , update ta TableCommut')
        {
            printf("TRAM TRANSFÉRÉ ! \n Je suis Switch MAC: ");
            printMACAddress(destination->data.m_switch.mac);
            printf(", j'ai reçu le message venant de MAC: ");
            printMACAddress(trame->source_mac);
            printf(" disant \"%s\"\n", trame->data);
            //printf("J'envoi un message en retour pour dire 'Je suis le desinataire , g bien reçus'\n");                                
            // current.MAC dans CommutTable ? 
            // --> foreach entry {
            //    if entry.MAC == current.MAC --> ne pas ajouter}
            //    else {addEntry(current.MAC , lastFreePort)} 

            
            
        }                   
        printf("FIN DU PROGRMAME Transfert \n");  
        return true;        
          
        
    } 
    else 
    {        
        // Sinon, transférer la trame aux sommets adjacents
        printf("Transfert_trame.Destination != trame.MAC_Dest --> transfert aux sommets adjacents de destation\n");
        sommet sommets_adjs[ordre(&r->m_graphe)];
        int numSommet = destination - r->m_equipements;
        printf("numSommet = %d \n", numSommet);
        int nb_adj = sommets_adjacents(&r->m_graphe, destination - r->m_equipements, sommets_adjs);
        printf("sommets adjacents de sommet %d (destination) : ");
        for (size_t j = 0; j < nb_adj; j++) 
        {
            printf("%d ", sommets_adjs[j]);
        }
        printf("\n");
        for (int i = 0; i < nb_adj; i++) // foreach equipement_sommet_adj
        {
            equipement *adj_equip = &r->m_equipements[sommets_adjs[i]];
            printf("Sommet adjacent %d ->  ",i); 
            if (adj_equip->type == STATION)
            {
                printf("station MAC :  ");
                printMACAddress(adj_equip->data.m_station.m_adresseMac); 
            }
            else
            {
                printf("switch MAC :  ");
                printMACAddress(adj_equip->data.m_switch.mac);
            }

            if (adj_equip == current) 
            {
                printf("C'est le sommet duquel j'ai reçus Trame , PASS \n");
            }
            else /*(adj_equip != current) */
            {
                printf("transfer trame de  ");
                if (current->type == STATION)
                {
                    printf(" STATION MAC: ");
                    printMACAddress(current->data.m_station.m_adresseMac);                
                }
                else if (current->type == SWITCH)
                {
                    printf(" SWITCH MAC: ");
                    printMACAddress(current->data.m_station.m_adresseMac);        
                }
                printf(" à ");                
                printf("Sommet adjacent '%d' de *destination->  ",i); 
                if (adj_equip->type == STATION)
                {
                    printf("station MAC :  ");
                    printMACAddress(adj_equip->data.m_station.m_adresseMac); 
                }
                else
                {
                    printf("switch MAC :  ");
                    printMACAddress(adj_equip->data.m_switch.mac);
                }

                if (transferer_trame(r, destination, adj_equip, trame)) {
                    // if destination.type == switch --> addSwitchTableEntry()
                    if (destination->type == SWITCH)
                    {
                        if (adj_equip->type == STATION)
                        {
                            int port = lastFreePort(destination);
                            printf("Trame transférée du Switch :");printMACAddress(destination->data.m_switch.mac);printf(" à STATION : ");printMACAddress(adj_equip->data.m_station.m_adresseMac);
                            printf("Vù que c pas dans TableCommut --> ajout de MAC adj_equip dans au dernier port dispo");
                            addSwitchTableEntry(destination,adj_equip->data.m_station.m_adresseMac, port);
                        }
                        else
                        {
                            printf("adj_equip est un autre Switch , je n'ajoute pas dans table Commut");
                        }                                                
                    }
                    
                    return true; // Arrêter la fonction si la trame a été transférée
                }
            }
        }
    }
    printf("après avoir vu tt les sommets , tjr pas réussi. ya pteter un prblm\n");
    return false; // Aucun transfert réussi
}

void envoyer_trame(reseau *r, trame_ethernet *trame) 
{
    printf("MAC_Dest de tram : ");
    printMACAddress(trame->destination_mac);
    // Trouver le sommet correspondant à l'adresse MAC source
    for (size_t i = 0; i < r->nb_equipements; i++) 
    {
        equipement *current = &r->m_equipements[i];
        if (compare_mac(current->data.m_station.m_adresseMac, trame->source_mac)) 
        {
            printf("Current MAC == j'ai trv le MAC SOURCE de trame :  ");
            printMACAddress(current->data.m_station.m_adresseMac);
            
            // Envoyer la trame aux sommets adjacents
            sommet sommets_adjs[ordre(&r->m_graphe)];
            int nb_adj = sommets_adjacents(&r->m_graphe, i, sommets_adjs);

            for (int j = 0; j < nb_adj; j++) 
            {
                equipement *adj_equip = &r->m_equipements[sommets_adjs[j]];
                printf("MAC du current sommet adj (sera le dest de transfer_trame)  : ");
                if (adj_equip->type == SWITCH)
                {
                    printMACAddress(adj_equip->data.m_switch.mac);
                }
                else if (adj_equip->type == STATION)
                {
                    printMACAddress(adj_equip->data.m_station.m_adresseMac);
                }                
                                
                if (transferer_trame(r, &r->m_equipements[i], adj_equip, trame)) {
                    return; // Arrêter la fonction si la trame a été transférée
                }
            }
            break; // Arrêter de chercher la source après l'avoir trouvée
            
            
        }
    }
}









int main() {
    
    reseau r;
    init_reseau(&r);
    lire_config("reseau_config.txt", &r);
    printReseau(&r);
    equipement sw0 = r.m_equipements[0];
    if (sw0.type == STATION)
    {
        printf("STATION\n");
    }
    else
    {
        printf("SWITCH\n");
    }
    equipement st1 = r.m_equipements[1];
    if (st1.type == STATION)
    {
        printf("STATION\n");
    }
    else
    {
        printf("SWITCH\n");
    }
    
    equipement st2 = r.m_equipements[2];
    equipement st3 = r.m_equipements[3];


    trame_ethernet trame;
    init_trame(&trame, r.m_equipements[1].data.m_station.m_adresseMac, r.m_equipements[2].data.m_station.m_adresseMac, "Hello, Station 2 from Station 1!");
    printTrame(&trame);
    // Envoyer une trame de la station 1 à la station 2
    envoyer_trame(&r, &trame);

    printReseau(&r);

    return 0;
    
}
