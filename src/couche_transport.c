#include <stdio.h>
#include "couche_transport.h"
#include "services_reseau.h"
#include "application.h"

/* ************************************************************************** */
/* *************** Fonctions utilitaires couche transport ******************* */
/* ************************************************************************** */

// RAJOUTER VOS FONCTIONS DANS CE FICHIER...

int generer_controle(paquet_t *pdata) {
    int somme = 0;
    somme ^= pdata->type;
    somme ^= pdata->lg_info;
    somme ^= pdata->num_seq;

    for (int i=0; i<pdata->lg_info; i++) {
        somme ^= pdata->info[i];
    }
    return somme;
}

bool verifier_controle(paquet_t *pdata) {
    return generer_controle(pdata)==pdata->somme_ctrl;
}

int inc(int num, int mod) {
    return (num+1)%mod;
}

/*--------------------------------------*/
/* Fonction d'inclusion dans la fenetre */
/*--------------------------------------*/
int dans_fenetre(unsigned int inf, unsigned int pointeur, int taille) {

    unsigned int sup = (inf+taille-1) % SEQ_NUM_SIZE;

    return
        /* inf <= pointeur <= sup */
        ( inf <= sup && pointeur >= inf && pointeur <= sup ) ||
        /* sup < inf <= pointeur */
        ( sup < inf && pointeur >= inf) ||
        /* pointeur <= sup < inf */
        ( sup < inf && pointeur <= sup);
}
