#include <stdio.h>
#include <stdint.h>

// Structure pour stocker une adresse IP
typedef struct {
    uint8_t octet[4];
} IPAddress;

// Fonction pour afficher une adresse IP
void printIPAddress(IPAddress ip) {
    printf("%d.%d.%d.%d", ip.octet[0], ip.octet[1], ip.octet[2], ip.octet[3]);
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

int main() {
    IPAddress ip, mask;

    // Demander à l'utilisateur de saisir l'adresse IP
    printf("Entrez l'adresse IP (format xxx.xxx.xxx.xxx): ");
    scanf("%hhu.%hhu.%hhu.%hhu", &ip.octet[0], &ip.octet[1], &ip.octet[2], &ip.octet[3]);

    // Demander à l'utilisateur de saisir le masque de sous-réseau
    printf("Entrez le masque de sous-reseau (format xxx.xxx.xxx.xxx): ");
    scanf("%hhu.%hhu.%hhu.%hhu", &mask.octet[0], &mask.octet[1], &mask.octet[2], &mask.octet[3]);

    // Appel de la fonction addnet
    addnet(ip, mask);

    return 0;
}