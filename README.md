
# Compiler et executer :
Suivre les instructions pour tester chacune des 3 étapes :
Dans le terminal : 
```bash
./make
```

Choisir lequel de ces 3 commandes executer  : 
```bash
./main
./main2
./main3
```

Pour nettoyer les fichiers générés: 

```bash
make clean
```
# Etape 1 : structure de donnée 

Le principe de cette étape est de créer la base de la structure réseau et du graphe qui est composée de stations(IP,MAC) et de switch(Port,MAC).

### Liste des structures :
IPAdress :
tableau de 4 octets(unite8_t)
Nous avons utilisé un tableau de 4 octets car une adresse ip en est composée de 4  qui caractérise les adresses IP d’un port ou d’un switch

MACAdress :
- tableau de 6 octets(unite8_t)
- une adresse MAC (constituée de 48 bites plus simplement dit 6 octets qui caractérise les adresse MAC d’un port ou d’un switch)

SwitchTableEntry (Entré pour la  table de commutation) :
  - MACAddress *tab_macAddresses;  // Tableau dynamique d'adresses MAC
  - int nb_mac;                // Nombre d'adresses MAC sur ce port
  - int capacite;              // Capacité du tableau d'adresses MAC
  - int m_port;
  - PortState port_state; // Ajout de l'état du port
  - BPDU bpdu;           // BPDU sur le port de la switchtableEntry

Relie une ou des adresses MAC pour un port

TableDeCommunication :
- capacité d'entrées (int)
- nombre actuel d'entrée  (int)
- tableau de (switchTableEntry)

Station :
- adresse MAC (Ethernet)
- adresse IP

Switch :
- adresse MAC (Ethernet)
- nombre de ports
- priorité (pour le protocole STP)
- table de communication 


Equipement :
- a un type
- data : qui peut représenter une station ou un switch. 


Réseau :
- graphe représentant le réseau
- Nombre d’équipements
- Liste d'équipement
- 


Reseau :
- graphe
- un tableau d'equipement
- taille du tableau equipement

Trame_ethernet :
    - tableau de 7 octets
  - Le SFD indique où commence la trame
  - Le MAC destinataire
  - Le MAC source
  - le type st utilisé pour indiquer le protocole de la couche supérieure auquel les données de la trame doivent être transmises


# Étape 2 : Stockage d’une architecture dans un fichier de configuration (main2)

L’étape 2 a pour but de lire un fichier texte externe qui donnera la configuration d'un réseau.

### Déroulement des fonctions

**lireMAC** transforme une adresse MAC texte en une adresse MAC

**lireIP** transforme une adresse IP texte en une adresse IP

**lire_config** prend le fichier mis en paramètre et le transforme en réseau

**init_reseau** initialise le réseau et le graphe vide

Cree une Sation ou un Switch par rapport à leur attribut puis , une fois crée , l'ajoute au réseau (en mettant à jour le grpahe)


# Étape 3 : Commutation de trames Ethernet (main3)

L'étape 3 a pour but d'envoyer des trames d'une adresse MAC source à une adresse MAC destination

**init_trame** qui crée une trame avec une adresse MAC source a une adresse MAC destination

**print_trame_ethernet_user** pour afficher la trame en mode utilisateur pour faciliter la representation de celle ci

**print_trame_ethernet_hex** pour afficher la trame en mode hexadecimal

**compare_mac** compare 2 adresse MAC pour savoir si c'est elles sont égales 

**get_equipement_by_mac** récupère un equipement par rapport à une adresse MAC dans le réseau 

**printTransferTrame** montre le transfere de la trame

 **transfer_trame** : simule l'envoi d'une trame d'un équipement à une autre.

Pour le cas des switch : si un switch ne sais pas sur quel port transferer la trame , alors il l'envoie a tout le monde et enrgistre le port d'où vient la trame dans sa table de commutation.
Pour le cas des stations : s'il n'est pas le destinataire, il répond qu'il n'est pas la destination.Sinon , dit son adresse MAC est disant qu'il l'est

**envoi_trame**  : simule l'envoi de trame d'une station vers une autre
# Étape 4 : STP

Pas d'implémentation complète ni tests, mais nous avions pensé : 

Structures de BPDU 
- id_root
- cout
- id_transmetteur

Structure Enum Port_state
- port_designe
- port_root
- port_bloque

Ajout d'un nouvel attribut BPDU à Switch ainsi que d'un tableau avec l'état de chacun de ses ports.



