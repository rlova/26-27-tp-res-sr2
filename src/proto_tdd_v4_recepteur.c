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
    int taille_fenetre = 4; /* par défaut */
    if (argc>2) {
        printf("Usage : %s <taille_fenetre>\n",argv[0]);
        return 1;
    }
    if (argc == 2) {
        taille_fenetre = atoi(argv[1]);
        if (taille_fenetre<1 || taille_fenetre> SEQ_NUM_SIZE/2) {
            printf("Erreur : taille fenêtre doit être comprise entre 1 et %d\n",SEQ_NUM_SIZE/2);
            return 1;
        }
    }
    // unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t pdata;                  /* paquet utilisé par le protocole */
    paquet_t pack;                   /* paquet pour stocker les acquittements */
    paquet_t tampon[SEQ_NUM_SIZE];  /* paquets recus hors s&quence */
    bool recu[SEQ_NUM_SIZE] = {false};  /* recu[n] = paquet n présent dans le tampon */
    int fin = 0;                     /* condition d'arrêt */
    int paquet_attendu = 0;

    pack.type = ACK;
    pack.lg_info = 0;
    // pack.num_seq = 15;

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&pdata);
        
        if (pdata.type == DATA && verifier_controle(&pdata)) {
            /* on acquitte le paquet */
            pack.type = ACK;
            pack.lg_info = 0;
            pack.num_seq = pdata.num_seq;
            pack.somme_ctrl = generer_controle(&pack);
            vers_reseau(&pack);
    
            /* si le paquet est dans la fenêtre (hors séquence ou pas ): on le bufferise */
            if (dans_fenetre(paquet_attendu,pdata.num_seq,taille_fenetre)) {
                tampon[pdata.num_seq] = pdata;
                recu[pdata.num_seq] = true;
            }

            while (!fin && recu[paquet_attendu]) {
                fin = vers_application(tampon[paquet_attendu].info,tampon[paquet_attendu].lg_info);
                recu[paquet_attendu] = false;
                paquet_attendu = inc(paquet_attendu,SEQ_NUM_SIZE);
            }
            // if (pdata.num_seq == paquet_attendu) {
            //     /* extraction des donnees du paquet recu */
            //     for (int i=0; i<pdata.lg_info; i++) {
            //         message[i] = pdata.info[i];
            //     }

            //     /* remise des données à la couche application */
            //     fin = vers_application(message, pdata.lg_info);

            //     pack.num_seq = paquet_attendu;  // on acquitte ce paquet
            //     paquet_attendu = inc(paquet_attendu,16); // le même que l'emetteur
            // }
            // /* hors séquence : pack.num_seq contient encore le dernier
            // paquet correct reçu, donc on le ré-acquitte */
            // pack.somme_ctrl = generer_controle(&pack);
            // vers_reseau(&pack);
        } 
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
