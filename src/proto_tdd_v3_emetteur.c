/*************************************************************
* proto_tdd_v3 -  émetteur                                   *
* TRANSFERT DE DONNEES  v3                                   *
*                                                            *
* Protocole avec contrôle de flux "Go-Back-N" ARQ            *
*                                                            *
*                                                            *
* Université de Toulouse / FSI / Dpt d'informatique          *
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    int taille_fenetre = 4; // par défaut
    if (argc>2) {
        printf("Usage : %s <taille_fenetre>\n",argv[0]);
        return 1;
    } 
    if (argc==2) {
        taille_fenetre = atoi(argv[1]);
        if (taille_fenetre<1 || taille_fenetre>=16) {
            printf("Erreur taille fenetre doit être inférieure à 16\n");
            return 1;
        }
    }
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg;                  /* taille du message */
    paquet_t tab_p[16];              /* tableau de paquet de taille 16 */    
    paquet_t pack;                   /* paquet utilisé pour les acquittements */
    int evt;
    int borne_inf = 0;
    int curseur = 0;

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 || borne_inf!=curseur ) {
        if (taille_msg!=0 && dans_fenetre(borne_inf,curseur,taille_fenetre)) {

            /* construction paquet */
            for (int i=0; i<taille_msg; i++) {
                tab_p[curseur].info[i] = message[i];
            }
            tab_p[curseur].lg_info = taille_msg;
            tab_p[curseur].type = DATA;
            tab_p[curseur].num_seq = curseur;
            tab_p[curseur].somme_ctrl = generer_controle(&tab_p[curseur]);

            /* remise à la couche reseau */
            vers_reseau(&tab_p[curseur]);

            if (borne_inf==curseur) {
                depart_temporisateur(100);
            }
            curseur = inc(curseur,16);
            de_application(message,&taille_msg);
        } else {
                evt = attendre();
                if (evt==PAQUET_RECU) { // 
                    de_reseau(&pack);
                    if (verifier_controle(&pack) && dans_fenetre(borne_inf,pack.num_seq,taille_fenetre)) {
                        borne_inf = inc(pack.num_seq,16);    // décale fenêtre de manière cumulative
                        if (borne_inf==curseur) {
                            arret_temporisateur();
                        }
                    }
                } else {
                    int i = borne_inf;
                    depart_temporisateur(100);
                    while (i!=curseur) {
                        vers_reseau(&tab_p[i]);
                        i = inc(i,16);
                    }
                }
            }
        /* lecture des donnees suivantes de la couche application */
        // de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
