/*************************************************************
* proto_tdd_v3 -  émetteur                                   *
* TRANSFERT DE DONNEES  v3                                   *
*                                                            *
* Protocole avec contrôle de flux "Selective Repeat" ARQ            *
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
        /* fenêtre max : la moitié de la capacité de numérotation */
        if (taille_fenetre<1 || taille_fenetre>=16) {
            printf("Erreur : taille fenetre doit être comprise entre 1 et %d\n",SEQ_NUM_SIZE/2);
            return 1;
        }
    }
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg;                  /* taille du message */
    paquet_t tab_p[SEQ_NUM_SIZE];              /* tableau de paquet de taille 16 */    
    bool acquitte[SEQ_NUM_SIZE] = {false};  /* acquitte[n] : paquet n déjà acquitté */
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
            acquitte[curseur] = false;

            /* remise à la couche reseau */
            vers_reseau(&tab_p[curseur]);

            depart_temporisateur_num(curseur,100);

            curseur = inc(curseur,SEQ_NUM_SIZE);

            de_application(message,&taille_msg);
        } else {
                evt = attendre();
                if (evt==PAQUET_RECU) { // 
                    de_reseau(&pack);
                    if (verifier_controle(&pack) && dans_fenetre(borne_inf,pack.num_seq,taille_fenetre)) {
                        /* on marque uniquement ce paquet (ACK individuel) */
                        acquitte[pack.num_seq] = true;
                        arret_temporisateur_num(pack.num_seq);
                        /* on décale que si le plus ancien est acquité 
                        
                        ensuite on traverse les autres paquets */
                        while (borne_inf!=curseur && acquitte[borne_inf]) {
                            acquitte[borne_inf] = false;
                            borne_inf = inc(borne_inf, SEQ_NUM_SIZE);
                        }
                    }
                } else {
                    /* time out, evt = num temporisateur expiré = num paquet */
                    vers_reseau(&tab_p[evt]);
                    depart_temporisateur_num(evt, 100);
                }
            }
        /* lecture des donnees suivantes de la couche application */
        // de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}