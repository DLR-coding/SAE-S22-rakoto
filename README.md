Pour compiler et executer :
-  aller danas le dossier s22 
- gcc -Wall s22.c graphe.c -o s22
- ./s22
( ce sera le main du fichier s22.c qui sera éxécuté)


# Etape 1 : structure de donnée

Le principe de cette étape est de créer la base de la structure réseau et du graph qui est composée de stations(IP,MAC)et  de switch(Port,MAC).
liste des structures : 
IPAdress : 
tableau de 4 octets(unite8_t)
on a utiliser un tableau de 4 octets car une adresse ip est composée 4
qui caractérise les adresse IP d’un port ou d’un switch 

MACAdress : 
   -tableau de 6 octets(unite8_t)
   -une adresse MAC est constituée de 48 bites plus simplement dit 6 octets
    qui caractérise les adresse MAC d’un port ou d’un switch
switchTableEntry (Entré pour la  table de commutation) :
   -nombre adresse MAC(MMACAdress)
   -nombre de port 
   -port par lequel il faut passer(int)
   -Table des état des port
   -Relie une adresse MAC pour un port

TableDeCommunication :
       
capacité de port(in
t)
       
nombre d'entrée  (int)
Adresse mac et leur port (switchTableEntry)

Station : 
      -adresse MAC (Ethernet)
      -adresse IP

Switch :
    -adresse MAC (Ethernet)
    -nombre de ports
    -priorité (pour le protocole STP)
    -table de communication qui garde les adresse MAC à un port


Equipement :
      
structure qui peut représenter soit une station,    soit un switch.
Sert à simuler un héritage.

Réseau : 
     -graphe représentant le réseau
     -Nombre d’équipement 
     -Liste d'équipement
     -Trame_ethernet :

Equipement : 
       -prend un type equipement qui choisie entre une sation et un switch

Reseau :
    
graphe 
un tableau d'equipement 
taille du tableau equipement

Trame_ethernet : 
        
tableau de 7 octets 
Le SFD indique où commence la trame
Le MAC destinataire 
Le MAC source
le type st utilisé pour indiquer le protocole de la couche supérieure auquel les données de la trame doivent être transmises


Étape 2 : Stockage d’une architecture dans un fichier de configuration

L’étape 2 a pour but de lire un fichier texte externe qui donnera les adesse MAC et les arêtes.

Déroulement des foncion
lireMAC transforme l'adresse MAC texte en une adresse MAC 

lireIP transforme l'adresse IP texte en une adresse IP

lire_config prend le fichier mis en paramentre et le transforme en réseau

init_reseau initalise le reseaux et le graph vide

Cree une Sationt ou un Switch par rapport a leur attribut 

puis une fois crée ajoute l'équipement 
,l'ajoute a un sommets et les sommets


Étape 3 : Commutation de trames Ethernet

L'étape 3 a pour but d'envoyer des trame d'une adresse MAC source a une adresse MAC destination

init_trame qui crée une trame avec une adresse MAC source a une adresse MAC destination

print_trame_ethernet_user pour afficher la trame en mode utilisateur pour faciliter la representation de celle ci

print_trame_ethernet_hex pour afficher la trame en mode hexadecimal

compare_mac compare 2 adresse MAC pour savoir si c'est la meme 

get_equipement_by_mac ajoute l'equipement par rapport a l'adresse MAC

printTransferTrame montre le transfere de la trame

transfer_trame il transfert la tram a la destination et si il ne la pas il l'envoie a tout le monde et enrgistre le port d'ou vient la trame dans le switch
si il n'est pas le destinataire repond qu'il n'est pas la destination et sinon dit son adresse MAC est disant que c'est lui