#include "s22.c"
#include "stdio.h"

int main() {
    reseau r;
    // Initialiser le réseau à partir du fichier de configuration
    lire_config("reseau_config.txt", &r);
    printReseau(r);

    equipement *sw0 = &r.tab_equipements[0];
    equipement *st1 = &r.tab_equipements[1];
    equipement *st2 = &r.tab_equipements[2];

    // Initialiser la trame
    trame_ethernet trame;
    init_trame(&trame, *get_mac_address(st1), *get_mac_address(st2), "message de st1 à st2");

    // Envoyer la trame
    envoi_trame(&r, st1, &trame);

    // Afficher la table de commutation après l'envoi de la trame
    printSwitch(sw0->data.m_switch);
    // Free mémoire
    freeReseau(&r);
    return 0;
}