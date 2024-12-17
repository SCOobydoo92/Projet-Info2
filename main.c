#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024
#define MAX_NODES 100000  // Taille maximale du tableau de nœuds

typedef struct NoeudAVL {
    int element;       // Identifiant de la station
    int equilibre;     // Facteur d'équilibre
    long capacite;     // Capacité de transfert ou production
    long consommation; // Somme des consommations des clients
    char type[10];     // Champ pour le type (HV-B, HV-A, LV)
    struct NoeudAVL *fg;
    struct NoeudAVL *fd;
} NoeudAVL, *Arbre;

// Fonction pour créer un nouveau nœud
Arbre creerArbre(int e, long capacite, long consommation, const char *type) {
    Arbre a = (Arbre)malloc(sizeof(NoeudAVL));
    if (a == NULL) {
        perror("Erreur d'allocation de mémoire pour le nœud");
        exit(EXIT_FAILURE); // Arrêt si allocation échoue
    }
    a->element = e;
    a->equilibre = 0;
    a->capacite = capacite;
    a->consommation = consommation;
    snprintf(a->type, sizeof(a->type), "%s", type);
    a->fg = NULL;
    a->fd = NULL;
    return a;
}

// Rotation droite
Arbre rotationDroite(Arbre a) {
    Arbre pivot = a->fg;
    a->fg = pivot->fd;
    pivot->fd = a;

    // Mise à jour des facteurs d'équilibre
    a->equilibre = (a->equilibre == -1) ? 0 : a->equilibre - 1;
    pivot->equilibre = (pivot->equilibre == -1) ? 0 : pivot->equilibre - 1;

    return pivot;
}

// Rotation gauche
Arbre rotationGauche(Arbre a) {
    Arbre pivot = a->fd;
    a->fd = pivot->fg;
    pivot->fg = a;

    // Mise à jour des facteurs d'équilibre
    a->equilibre = (a->equilibre == 1) ? 0 : a->equilibre + 1;
    pivot->equilibre = (pivot->equilibre == 1) ? 0 : pivot->equilibre + 1;

    return pivot;
}

// Rotation double droite
Arbre doubleRotationDroite(Arbre a) {
    a->fg = rotationGauche(a->fg);
    return rotationDroite(a);
}

// Rotation double gauche
Arbre doubleRotationGauche(Arbre a) {
    a->fd = rotationDroite(a->fd);
    return rotationGauche(a);
}

// Fonction pour insérer un élément dans un AVL
Arbre insertionAVL(Arbre a, int e, long capacite, long consommation, const char *type, int *h) {
    if (a == NULL) {
        *h = 1;
        return creerArbre(e, capacite, consommation, type);
    }

    if (e < a->element) {
        a->fg = insertionAVL(a->fg, e, capacite, consommation, type, h);
        if (*h) {
            a->equilibre--;
        }
    } else if (e > a->element) {
        a->fd = insertionAVL(a->fd, e, capacite, consommation, type, h);
        if (*h) {
            a->equilibre++;
        }
    } else {
        *h = 0;
        return a;
    }

    // Équilibrage après insertion
    if (a->equilibre < -1) {
        if (a->fg->equilibre <= 0) {
            a = rotationDroite(a);
        } else {
            a = doubleRotationDroite(a);
        }
        *h = 0;
    } else if (a->equilibre > 1) {
        if (a->fd->equilibre >= 0) {
            a = rotationGauche(a);
        } else {
            a = doubleRotationGauche(a);
        }
        *h = 0;
    } else if (a->equilibre == 0) {
        *h = 0;
    }

    return a;
}

// Fonction pour lire un fichier CSV avec filtrage
void lireFichierCSV(const char *nomFichier, Arbre *arbre, const char *typeStation, const char *typeConsommateur, int idCentrale) {
    FILE *fichier = fopen(nomFichier, "r");
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier CSV");
        exit(EXIT_FAILURE);
    }

    char ligne[MAX_LINE_LENGTH];
    while (fgets(ligne, sizeof(ligne), fichier)) {
        int idStation;
        int idParentCentrale = -1;
        long capacite = 0, consommation = 0;
        char type[10] = "";

        // Extraction des données selon le format attendu
        if (sscanf(ligne, "%d;%d;%ld;%ld;%s", &idStation, &idParentCentrale, &capacite, &consommation, type) != 5) {
            fprintf(stderr, "Erreur dans la ligne : %s\n", ligne);
            continue;
        }

        // Appliquer les filtres utilisateur
        if (typeStation != NULL && strcmp(type, typeStation) != 0) {
            continue; // Type de station non correspondant
        }
        if (idCentrale != -1 && idParentCentrale != idCentrale) {
            continue; // ID de centrale non correspondant
        }
        if (typeConsommateur != NULL) {
            if (strcmp(typeConsommateur, "comp") == 0 && consommation == 0) {
                continue; // On cherche des entreprises (consommation non vide)
            }
            if (strcmp(typeConsommateur, "indiv") == 0 && consommation == 0) {
                continue; // On cherche des particuliers
            }
        }

        int hauteur = 0;
        *arbre = insertionAVL(*arbre, idStation, capacite, consommation, type, &hauteur);
    }

    fclose(fichier);
}

// Fonction pour analyser les surproductions/sous-productions
void analyserStationsTriees(Arbre a, FILE *sortie, Arbre **tableau, int *index) {
    if (a != NULL) {
        analyserStationsTriees(a->fg, sortie, tableau, index);
        tableau[*index] = a;
        (*index)++;
        analyserStationsTriees(a->fd, sortie, tableau, index);
    }
}

// Fonction pour trier et écrire les stations dans le fichier de sortie
void ecrireStationsTriees(Arbre a, FILE *sortie) {
    int nombreNoeuds = 0;
    Arbre *tableau = malloc(MAX_NODES * sizeof(Arbre));
    if (tableau == NULL) {
        perror("Erreur d'allocation de mémoire pour le tableau des stations");
        return;
    }

    analyserStationsTriees(a, sortie, tableau, &nombreNoeuds);

    // Vérification du dépassement de mémoire
    if (nombreNoeuds > MAX_NODES) {
        fprintf(stderr, "Trop de stations pour allouer suffisamment de mémoire\n");
        free(tableau);
        return;
    }

    // Trier par capacité croissante
    for (int i = 0; i < nombreNoeuds - 1; i++) {
        for (int j = 0; j < nombreNoeuds - i - 1; j++) {
            if (tableau[j]->capacite > tableau[j + 1]->capacite) {
                Arbre temp = tableau[j];
                tableau[j] = tableau[j + 1];
                tableau[j + 1] = temp;
            }
        }
    }

    // Écrire dans le fichier de sortie
    for (int i = 0; i < nombreNoeuds; i++) {
        Arbre station = tableau[i];
        long difference = station->capacite - station->consommation;
        fprintf(sortie, "%d:%ld:%ld:%s\n", station->element, station->capacite, station->consommation, difference >= 0 ? "Surproduction" : "Sous-production");
    }

    free(tableau);
}

// Fonction pour extraire les 10 stations LV les plus et moins chargées
void extraireStationsExtremes(Arbre a, const char *nomFichier) {
    int nombreNoeuds = 0;
    Arbre *tableau = malloc(MAX_NODES * sizeof(Arbre));
    if (tableau == NULL) {
        perror("Erreur d'allocation de mémoire pour le tableau des stations extrêmes");
        return;
    }

    FILE *fichierMinMax = fopen(nomFichier, "w");
    if (!fichierMinMax) {
        perror("Erreur lors de l'ouverture du fichier min/max");
        free(tableau);
        return;
    }

    analyserStationsTriees(a, NULL, tableau, &nombreNoeuds);

    // Vérification du dépassement de mémoire
    if (nombreNoeuds > MAX_NODES) {
        fprintf(stderr, "Trop de stations pour allouer suffisamment de mémoire\n");
        free(tableau);
        fclose(fichierMinMax);
        return;
    }

    // Trier par consommation décroissante pour extraire les extrêmes
    for (int i = 0; i < nombreNoeuds - 1; i++) {
        for (int j = 0; j < nombreNoeuds - i - 1; j++) {
            if (labs(tableau[j]->capacite - tableau[j]->consommation) < labs(tableau[j + 1]->capacite - tableau[j + 1]->consommation)) {
                Arbre temp = tableau[j];
                tableau[j] = tableau[j + 1];
                tableau[j + 1] = temp;
            }
        }
    }

    // Écrire les 10 plus chargées
    fprintf(fichierMinMax, "Stations les plus chargées:\n");
    for (int i = 0; i < 10 && i < nombreNoeuds; i++) {
        Arbre station = tableau[i];
        fprintf(fichierMinMax, "%d:%ld:%ld\n", station->element, station->capacite, station->consommation);
    }

    // Écrire les 10 moins chargées
    fprintf(fichierMinMax, "\nStations les moins chargées:\n");
    for (int i = nombreNoeuds - 1; i >= nombreNoeuds - 10 && i >= 0; i--) {
        Arbre station = tableau[i];
        fprintf(fichierMinMax, "%d:%ld:%ld\n", station->element, station->capacite, station->consommation);
    }

    fclose(fichierMinMax);
    free(tableau);
}

// Fonction pour libérer un arbre AVL
void libererArbre(Arbre a) {
    if (a == NULL) return;
    libererArbre(a->fg);
    libererArbre(a->fd);
    free(a);
}

// Fonction principale
int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <fichier_csv> <fichier_sortie> <type_station> <type_consommateur> [id_centrale]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *fichierCSV = argv[1];
    const char *fichierSortie = argv[2];
    const char *typeStation = argv[3];
    const char *typeConsommateur = argv[4];
    int idCentrale = (argc == 6) ? atoi(argv[5]) : -1;

    printf("Lecture du fichier CSV...\n");
    Arbre arbre = NULL;
    lireFichierCSV(fichierCSV, &arbre, typeStation, typeConsommateur, idCentrale);

    printf("Ouverture du fichier de sortie...\n");
    FILE *sortie = fopen(fichierSortie, "w");
    if (sortie == NULL) {
        perror("Erreur lors de l'ouverture du fichier de sortie");
        libererArbre(arbre);
        return EXIT_FAILURE;
    }

    fprintf(sortie, "Station:Capacite:Consommation:Etat\n");
    ecrireStationsTriees(arbre, sortie);
    fclose(sortie);

    if (strcmp(typeStation, "lv") == 0 && strcmp(typeConsommateur, "all") == 0) {
        printf("Extraction des stations extrêmes...\n");
        extraireStationsExtremes(arbre, "lv_all_minmax.csv");
    }

    libererArbre(arbre);
    printf("Analyse terminée. Résultats dans %s\n", fichierSortie);
    return EXIT_SUCCESS;
}

    libererArbre(arbre);
    printf("Analyse terminée. Résultats dans %s\n", fichierSortie);
    return EXIT_SUCCESS;
}
