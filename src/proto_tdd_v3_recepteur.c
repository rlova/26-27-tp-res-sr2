/*************************************************************
* proto_tdd_v3 -  récepteur                                  *
* TRANSFERT DE DONNEES  v3                                   *
*                                                            *
* Protocole avec contrôle de flux "Go-Back-N" ARQ            *
*                                                            *
*                                                            *
* Université de Toulouse / FSI / Dpt d'informatique          *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t pdata;                  /* paquet utilisé par le protocole */
    paquet_t pack;                   /* paquet pour stocker les acquittements */
    int fin = 0;                     /* condition d'arrêt */
    int paquet_attendu = 0;

    /* ACK du dernier paquet correct reçu : avant le 1er paquet,
    on met 7 (hors fenêtre de l'émetteur, donc ignoré) */
    pack.type = ACK;
    pack.lg_info = 0;
    pack.num_seq = 15;

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&pdata);
        
        if (verifier_controle(&pdata)) {
            if (pdata.num_seq == paquet_attendu) {
                /* extraction des donnees du paquet recu */
                for (int i=0; i<pdata.lg_info; i++) {
                    message[i] = pdata.info[i];
                }

                /* remise des données à la couche application */
                fin = vers_application(message, pdata.lg_info);

                pack.num_seq = paquet_attendu;  // on acquitte ce paquet
                paquet_attendu = inc(paquet_attendu,16); // le même que l'emetteur
            }
            /* hors séquence : pack.num_seq contient encore le dernier
            paquet correct reçu, donc on le ré-acquitte */
            pack.somme_ctrl = generer_controle(&pack);
            vers_reseau(&pack);
        } // on ignore un paquet reçu avec erreur
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
