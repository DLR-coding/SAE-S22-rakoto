#include "test4.c"
#include "stdio.h"

int main() {
    reseau r;
    lire_config("reseau_config.txt", &r);
    printReseau(r);

    // Free mémoire
    freeReseau(&r);
    return 0;
}
