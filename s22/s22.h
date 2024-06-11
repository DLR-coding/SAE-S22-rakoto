#pragma once

#include "graphe.h"
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

void setPortState(Switch *sw, int port, PortState state);

void initBPDU(BPDU *bpdu, MACAddress root_id, uint32_t cost, MACAddress transmitting_id);

void printIPAddress(IPAddress ip);

void printMACAddress(MACAddress mac);
void printSwitchTableEntry(switchTableEntry entry) ;
void printSwitchTable(TableDeCommutation table);
void printSwitch(Switch sw);
void printStation(station s);
void printEquipement(equipement equip);
void printReseau(reseau r);
void initTableDeCommutation(TableDeCommutation *table, int capacite, MACAddress MACswitch) ;
int trouverPortPourMAC(TableDeCommutation *table, MACAddress mac);
switchTableEntry* rechercherEntreeParPort(TableDeCommutation *table, int port);
void ajouterMacAUneEntree(switchTableEntry *entree, MACAddress mac);
void ajouterEntree(TableDeCommutation *table, MACAddress mac, int port, MACAddress root_id, uint32_t cost, MACAddress transmitting_id);
void initSwitch(Switch *sw, MACAddress mac, int nbport, int priorite);
MACAddress* get_mac_address(equipement *equipment);
void init_reseau(reseau *r) ;
void ajouter_equipement(reseau *r, equipement equip);
void freeSwitch(Switch *s);
void freeReseau(reseau *r);

void read_mac_address(const char *str, MACAddress *mac);
void read_ip_address(const char *str, IPAddress *ip);
void lireMAC(char *str, MACAddress *mac);
void lireIP(char *str, IPAddress *ip);
void lire_config(const char *filename, reseau *r);
void init_trame(trame_ethernet *trame , MACAddress MAC_src, MACAddress MAC_dest , const char *message);
void print_trame_ethernet_user(trame_ethernet trame);
void print_trame_ethernet_hex(trame_ethernet trame);
bool compare_mac(MACAddress mac1, MACAddress mac2);
equipement *get_equipement_by_mac(reseau *r, MACAddress mac);
void printTransferTrame(equipement src , equipement dest);
void transfer_trame(reseau *r, equipement *source, equipement *destinataire, trame_ethernet *trame);
void envoi_trame(reseau *r, equipement *stationsource, trame_ethernet *trame);
