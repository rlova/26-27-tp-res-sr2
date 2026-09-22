/*************************************************************
* proto_tdd_v1 -  récepteur                                  *
* TRANSFERT DE DONNEES  v1                                   *
*                                                            *
* Protocole avec contrôle de flux "Stop-and-Wait",           *
* avec détection d'erreurs                                   *
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

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&pdata);
        
        if (verifier_controle(&pdata)) {

            /* extraction des donnees du paquet recu */
            for (int i=0; i<pdata.lg_info; i++) {
                message[i] = pdata.info[i];
            }

            /* remise des données à la couche application */
            fin = vers_application(message, pdata.lg_info);

            pack.type = ACK;
        } else {
            pack.type = NACK;
        }

        vers_reseau(&pack);
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
