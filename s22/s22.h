/*#ifndef GRAPHE_H
#define GRAPHE_H
#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "graph.h"

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

void initSwitch(Switch *sw, MACAddress MAC, int nbport, int priorite, int capacite) ;
void addSwitchTableEntry(Switch *sw, MACAddress mac, int port) ;
void printIPAddress(IPAddress ip) ;
void bitsToMACAddress(uint8_t bits[48], MACAddress *mac) ;
void printMACAddress(MACAddress mac) ;
void printSwitchTable(Switch *sw) ;
bool reseau_ajouter_connexion(reseau *r, size_t indice_eqt1, size_t indice_eqt2) ;
void addnet(IPAddress ip, IPAddress mask) ;
bool lire_config(const char *nomFichier, reseau *r) ;
#endif // GRAPHE_H
*/