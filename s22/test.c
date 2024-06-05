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

typedef struct {
    int numport; // = id
    enum etat_port { INCONNU, BLOQUE, ECoute, APPRENTISSAGE, TRANSMISSION } etat;
    int cout;
    MACAddress voisin;
    bool port_racine;
    bool port_designe;
} port;


// switch
typedef struct Switch
{
    MACAddress mac;
    int nbport;
    port *ports;
    int priorite;
    int BID; //  = (priorite & mac)
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
    equipement* m_equipements;  size_t nb_equipements;
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








//////////////////
/////////// Partie 3



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

///////////////////
    
/*                  MAIN
   // Partie 3
   // Créer une structure de réseau vide
    reseau r ;

    // Lire le fichier de configuration et créer le réseau
    lire_config("mylan_no_cycle.txt", &r);
    
    
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
    for(int i = 0; i < 10; i++) { // on remplit les 10 premiers octets de donnees avec des valeurs arbitraires
        trame.data[i] = i;
    }
    for(int i = 0; i < 10; i++) { // on remplit les 10 premiers octets de donnees avec des valeurs arbitraires
        trame.data[i] = i;
    }
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

    return 0;
    */