/*
 * INF3172
 * programme simulant les algorithmes d’ordonnancement des processus SJF, SJFP et RR.
 *
 * Auteur : Mathieu Pronovost PROM18118300
 * Version : 24 février 2013
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define TAILLE_MAX 100 // Longeur du tableau

int main (int argc, char *argv []) {
    FILE *fichier;
    int j , i; // Sera utilisé dans les boucles for
    int processus[TAILLE_MAX][TAILLE_MAX]; // Contiendra tous les nombres du fichier en argument
    int nbProcessus = 0; // Nombre de processus
    char chaine[TAILLE_MAX] = "";
    char *data = chaine;
    int etape[TAILLE_MAX];
    int etapeFinale[TAILLE_MAX];
    int nbEntiers = 0;
    int quantum = atoi(argv[2]);
    char resultat[10000] = "";

    if ( (fichier = fopen(argv[1], "r")) == NULL ) {
        printf("Impossible d'ouvrir %s", argv[1]);
        exit(1);
    }

    /* Prend tous les nombres contenu dans le fichier en argument et
       les insère dans processus[]. */
    i = 0;
    j = 0;
    int offset = 0;
    while (fgets(chaine, TAILLE_MAX, fichier) != NULL) {
        while (sscanf(data, "%d%n", &processus[j][i], &offset) == 1) {
            data += offset;
            i++;
            nbEntiers++;
        }
        nbProcessus++;
        etape[j] = 2;
        etapeFinale[j] = nbEntiers - 1;
        nbEntiers = 0;
        j++;
        i = 0;
        data = chaine;
    }
    //processus[i] = 8888;

    // Variables utilisées pour les ordonnancements
    i = 0; // Nombre de processus terminés
    j = 0; // Temps
    int a = 0;
    int b = 0;
    int k = 0;
    int PID = 0;
    int min = 0;
    int prochainProcessus = 0;

    // Variables utilisées pour les forks et les pipes
    int fr1, fr2, fr3, lire, parler;
    int filedesFP1[2];
    int filedesFP2[2];
    int filedesFP3[2];
    char * phrase;
    phrase = (char *) malloc(12000);

    // Création des pipes
    if  (pipe( filedesFP1)==-1) {
        perror("filedesFP creation");
        abort();
    }
    if  (pipe( filedesFP2)==-1) {
        perror("filedesFP creation");
        abort();
    }
    if  (pipe( filedesFP3)==-1) {
        perror("filedesFP creation");
        abort();
    }

    /* Création de 1 père ayant 3 fils. Chaque fils a un algorithme d'ordonnancement
       et envoie le résultat au père */
    fr1 = fork();
    if (fr1 == 0) {
        // SJF
        sprintf(resultat, "Ordonnancement des processus selon l'algorithme : SJF\n");
        while (i < nbProcessus) {
            min = -1;
            for (k = 0; k < nbProcessus; k++) {
                if (etape[k] >= 0) {
                    if (processus[k][1] <= j) {
                        if ((min == -1) || (processus[k][etape[k]] < min)) {
                            if (processus[k][etape[k]] < 0) {
                                processus[k][1] = j - processus[k][etape[k]];
                                if (etape[k] < etapeFinale[k]) {
                                    etape[k]++;
                                } else {
                                    etape[k] = -1;
                                    i++;
                                }
                            } else {
                                prochainProcessus = k;
                                min = processus[k][etape[k]];
                            }
                        }
                    }
                }
            }
            if (min == -1) {
                j++;
                if (PID == 0) {
                    b = j;
                } else {
                    sprintf(resultat, "%sPID %d : %d-%d\n", resultat, PID, a, b);
                    a = b;
                    b = j;
                    PID = 0;
                }
            } else {
                j = j + min;
                if (etape[prochainProcessus] < etapeFinale[prochainProcessus]) {
                    etape[prochainProcessus]++;
                } else {
                    etape[prochainProcessus] = -1;
                    i++;
                }
                if (PID == processus[prochainProcessus][0]) {
                    b = j;
                } else {
                    if (PID == 0) {
                        if (a != 0 || b != 0) {
                            sprintf(resultat, "%sIDLE : %d-%d\n", resultat, a, b);
                        }
                    } else {
                        sprintf(resultat, "%sPID %d : %d-%d\n", resultat, PID, a, b);
                    }
                    a = b;
                    b = j;
                    PID = processus[prochainProcessus][0];
                }
            }
        }
        sprintf(resultat, "%sPID %d : %d-%d\n", resultat, PID, a, b);
        parler=filedesFP1[1];
        write(parler,resultat,4000); // Envoi du résultat vers le père

    } else {
        fr2 = fork();
        if (fr2 == 0) {
            // SJFP
            sprintf(resultat, "Ordonnancement des processus selon l'algorithme : SJFP\n");
            while (i < nbProcessus) {
                min = -1;
                for (k = 0; k < nbProcessus; k++) {
                    if (etape[k] >= 0) {
                        if (processus[k][1] <= j) {
                            if ((min == -1) || (processus[k][etape[k]] < min)) {
                                if (processus[k][etape[k]] < 0) {
                                    processus[k][1] = j - processus[k][etape[k]];
                                    if (etape[k] < etapeFinale[k]) {
                                        etape[k]++;
                                    } else {
                                        etape[k] = -1;
                                        i++;
                                    }
                                } else {
                                    prochainProcessus = k;
                                    min = processus[k][etape[k]];
                                }
                            }
                        }
                    }
                }
                if (min == -1) {
                    j++;
                    if (PID == 0) {
                        b = j;
                    } else {
                        sprintf(resultat, "%sPID %d : %d-%d\n", resultat, PID, a, b);
                        a = b;
                        b = j;
                        PID = 0;
                    }
                } else {
                    j++;
                    if (min == 0) {
                        if (etape[prochainProcessus] < etapeFinale[prochainProcessus]) {
                            etape[prochainProcessus]++;
                        } else {
                            etape[prochainProcessus] = -1;
                            i++;
                        }
                    } else {
                        processus[prochainProcessus][etape[prochainProcessus]]--;
                    }
                    if (PID == processus[prochainProcessus][0]) {
                        b = j;
                    } else {
                        if (PID == 0) {
                            if (a != 0 || b != 0) {
                                sprintf(resultat, "%sIDLE : %d-%d\n", resultat, a, b);
                            }
                        } else {
                            sprintf(resultat, "%sPID %d : %d-%d\n", resultat, PID, a, b);
                        }
                        j--;
                        a = b;
                        b = j;
                        PID = processus[prochainProcessus][0];
                    }
                }
            }
            sprintf(resultat, "%sPID %d : %d-%d\n", resultat, PID, a, b);
            parler=filedesFP2[1];
            write(parler,resultat,4000);

        } else {
            fr3 = fork();
            if (fr3 == 0) {
                // RR
                sprintf(resultat, "Ordonnancement des processus selon l'algorithme : RR %d\n", quantum);
                int quantumInitial = quantum;
                while (i < nbProcessus) {
                    min = -1;
                    for (k = 0; k < nbProcessus; k++) {
                        if (etape[k] >= 0) {
                            if (processus[k][1] <= j) {
                                if (processus[k][etape[k]] < 0) {
                                    processus[k][1] = j - processus[k][etape[k]];
                                    if (etape[k] < etapeFinale[k]) {
                                        etape[k]++;
                                    } else {
                                        etape[k] = -1;
                                        i++;
                                    }
                                } else {
                                    min = 1;
                                    while (quantum > 0) {
                                        if (processus[k][etape[k]] <= quantum) {
                                            j = j + processus[k][etape[k]];
                                            quantum -= processus[k][etape[k]];
                                            if (etape[k] < etapeFinale[k]) {
                                                etape[k] += 1;
                                                if (processus[k][etape[k]] < 0) {
                                                    processus[k][1] = j - processus[k][etape[k]];
                                                    quantum = -1;
                                                    if (etape[k] < etapeFinale[k]) {
                                                        etape[k]++;
                                                    } else {
                                                        etape[k] = -1;
                                                        i++;
                                                        quantum = -1;
                                                    }
                                                }
                                            } else {
                                                etape[k] = -1;
                                                i++;
                                                quantum = -1;
                                            }
                                        } else {
                                            j += quantum;
                                            processus[k][etape[k]] -= quantum;
                                            quantum = -1;
                                        }
                                    }
                                    quantum = quantumInitial;
                                    if (PID == processus[k][0]) {
                                        b = j;
                                    } else {
                                        if (PID == 0) {
                                            sprintf(resultat, "%sIDLE : %d-%d\n", resultat, a, b);
                                        } else {
                                            sprintf(resultat, "%sPID %d : %d-%d\n", resultat, PID, a, b);
                                        }
                                        a = b;
                                        b = j;
                                        PID = processus[k][0];
                                    }
                                }
                            }
                        }
                    }
                    if (min == -1) {
                        j++;
                        if (PID == 0) {
                            b = j;
                        } else {
                            printf("PID %d : %d-%d\n", PID, a, b);
                            a = b;
                            b = j;
                            PID = 0;
                        }
                    }
                }
                sprintf(resultat, "%sPID %d : %d-%d\n", resultat, PID, a, b);
                parler=filedesFP3[1];
                write(parler,resultat,4000);

            } else {
                // Lecture des trois résultats par le père
                lire=filedesFP1[0];
                read(lire,phrase,4000);
                puts(phrase);
                wait(&fr1);

                lire=filedesFP2[0];
                read(lire,phrase,4000);
                puts(phrase);
                wait(&fr2);

                lire=filedesFP3[0];
                read(lire,phrase,4000);
                puts(phrase);
                wait(&fr3);
            }
        }
    }
    fclose(fichier);
    return 0;
}
