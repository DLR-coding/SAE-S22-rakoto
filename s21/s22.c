#include <stdio.h>
#include <stdint.h>
#include <graphe.h>

// Structure pour stocker une adresse IP
typedef struct IPAddress{
    uint8_t octet[4];
} IPAddress;

typedef struct MACAddress{
    uint8_t octets[6];
} MACAddress;

/*Pour cela, vous aurez tout d’abord besoin de créer des types permettant : 
de représenter des adresses Ethernet et IPv4 
L’adresse IP ne servira ici qu’à identifier les stations dans le réseau puisqu’il n’est pas demandé de modéliser l’algorithme de routage IP.
*/

// switch
typedef struct Switch
{
    MACAddress MAC;
    int nbport;
    int priorite;
    switchTableEntry *table; 
} Switch; 

// pour crer un switch 
void initSwitch(Switch *sw, MACAddress mac, int nbport, int priorite, int initialCapacity) {
    sw->MAC = mac;
    sw->nbport = nbport;
    sw->priorite = priorite;
    sw->table = (switchTableEntry *)malloc(sizeof(switchTableEntry) * initialCapacity);
    if (!sw->table) {
        perror("Failed to allocate memory for switch table");
        exit(EXIT_FAILURE);
    }
}

//pr ajouter une entrée
void addSwitchTableEntry(Switch *sw, switchTableEntry *table, int *size, int *capacity, MACAddress mac, int port) {
    if (*size == *capacity) {
        // Redimensionner le tableau si nécessaire
        *capacity *= 2;
        table = (switchTableEntry *)realloc(table, sizeof(switchTableEntry) * (*capacity));
        if (!table) {
            perror("Failed to reallocate memory for switch table");
            exit(EXIT_FAILURE);
        }
    }
    table[*size].macAddress = mac;
    table[*size].port = port;
    (*size)++;
    sw->table = table; // Mise à jour du pointeur de la table dans le switch
}


typedef struct switchTableEntry {
    MACAddress macAddress;
    int port;
} switchTableEntry;


// Structure de la station
typedef struct station{
    IPAddress adresseIP;
    MACAddress adresseMac;
}station;


typedef struct reseau 
{
    graphe g;
    Switch *s;
    station *station; 
    int nbMachine;
} reseau;



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


void printSwitchTable(Switch *sw, int size) {
    printf("Switch Table:\n");
    for (int i = 0; i < size; i++) {
        printMACAddress(sw->table[i].macAddress);
        printf(" -> Port %d\n", sw->table[i].port);
    }
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
void lire_config(const char *nomFichier, structReseau *reseau) {
    
    fscanf(file, "%d %d", &reseau->numSwitch, &reseau->numStation);

    reseau->switch = malloc(reseau->numSwitch * sizeof(Switch));
    reseau->stations = malloc(reseau->numStation * sizeof(Station));
    reseau->liens = malloc(reseau->numSwitch * sizeof(int *));

    for (int i = 0; i < reseau->numSwitch; i++) {
        reseau->liens[i] = malloc(3 * sizeof(int));
        fscanf(file, "2;%17s;%d;%d", reseau->switch[i].mac, &reseau->switch[i].ports, &reseau->switch[i].priorite);
    }

    for (int i = 0; i < reseau->numStation; i++) {
        fscanf(file, "1;%17s;%15s", reseau->stations[i].mac, reseau->stations[i].ip);
    }

    for (int i = 0; i < reseau->numLiens; i++) {
        fscanf(file, "%d;%d;%d", &reseau->liens[i][0], &reseau->liens[i][1], &reseau->liens[i][2]);
    }

    fclose(file);
    }
}

/////////////////////////////////////////////////////:
 
int main() {
    // Créer une adresse MAC pour le switch
    MACAddress mac = {{0x00, 0x11, 0x22, 0x33, 0x44, 0x55}}};

    // Créer un switch avec 8 ports et une priorité de 32
    Switch sw;
    int initialCapacity = 4;
    initSwitch(&sw, mac, 8, 32, initialCapacity);

    // Ajouter des entrées à la table de commutation du switch
    MACAddress mac1 = {{0x00, 0x01, 0x02, 0x03, 0x04, 0x05}}};
    addSwitchTableEntry(&sw, sw.table, &sw.tableSize, &sw.tableCapacity, mac1, 1);

    MACAddress mac2 = {{0x00, 0x10, 0x11, 0x12, 0x13, 0x14}}};
    addSwitchTableEntry(&sw, sw.table, &sw.tableSize, &sw.tableCapacity, mac2, 2);

    MACAddress mac3 = {{0x00, 0x20, 0x21, 0x22, 0x23, 0x24}}};
    addSwitchTableEntry(&sw, sw.table, &sw.tableSize, &sw.tableCapacity, mac3, 3);

    // Afficher les informations du switch
    printf("Adresse MAC du switch : ");
    for (int i = 0; i < 6; i++) {
        printf("%02x", sw.MAC.octets[i]);
        if (i < 5) {
            printf(":");
        }
    }
    printf("\n");

    printf("Nombre de ports : %d\n", sw.nbport);

    printf("Priorité : %d\n", sw.priorite);

    printf("Table de commutation :\n");
    printf("Adresse MAC          Port\n");
    printf("---------------------------------\n");
    for (int i = 0; i < sw.tableSize; i++) {
        printf("%02x:%02x:%02x:%02x:%02x:%02x    %d\n",
               sw.table[i].macAddress.octets[0],
               sw.table[i].macAddress.octets[1],
               sw.table[i].macAddress.octets[2],
               sw.table[i].macAddress.octets[3],
               sw.table[i].macAddress.octets[4],
               sw.table[i].macAddress.octets[5],
               sw.table[i].port);
    }

    // Libérer la mémoire allouée pour la table de commutation
    free(sw.table);

    return 0;
}