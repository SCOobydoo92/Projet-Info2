#!/bin/bash

# Vérification des paramètres obligatoires
if [ "$#" -lt 3 ]; then
    echo "Usage: $0 <chemin_fichier_csv> <type_station> <type_consommateur> [<identifiant_centrale>]"
    exit 1
fi

# Assignation des paramètres
chemin_fichier_csv=$1
type_station=$2
type_consommateur=$3
identifiant_centrale=${4:--1}  # Par défaut, -1 indique aucun identifiant

# Vérification des types de station et consommateur
if [[ "$type_station" != "hvb" && "$type_station" != "hva" && "$type_station" != "lv" ]]; then
    echo "Erreur: type de station invalide. Valeurs possibles: hvb, hva, lv"
    exit 1
fi

if [[ "$type_consommateur" != "comp" && "$type_consommateur" != "indiv" && "$type_consommateur" != "all" ]]; then
    echo "Erreur: type de consommateur invalide. Valeurs possibles: comp, indiv, all"
    exit 1
fi

if [[ ("$type_station" == "hvb" || "$type_station" == "hva") && "$type_consommateur" != "comp" ]]; then
    echo "Erreur: seules les entreprises sont connectées aux stations HV-B et HV-A"
    exit 1
fi

# Vérification de l'existence du fichier CSV
if [ ! -f "$chemin_fichier_csv" ]; then
    echo "Erreur: le fichier CSV spécifié n'existe pas ou n'est pas accessible."
    exit 1
fi

# Vérification de la présence de l'exécutable C
if [ ! -f ./codeC/cwire ]; then
    echo "Compilation du programme C..."
    make -C ./codeC
    if [ $? -ne 0 ]; then
        echo "Erreur de compilation du programme C"
        exit 1
    fi
fi

# Création des dossiers tmp et graphs si nécessaires
mkdir -p tmp graphs

# Vider le dossier tmp
rm -rf tmp/*

# Lancer le traitement
temp_output="tmp/$(basename "$chemin_fichier_csv" .csv)_output.csv"
start_time=$(date +%s)
./codeC/cwire "$chemin_fichier_csv" "$temp_output" "$type_station" "$type_consommateur" "$identifiant_centrale"
if [ $? -ne 0 ]; then
    echo "Erreur lors de l'exécution du programme C."
    exit 1
fi
end_time=$(date +%s)

# Calcul et affichage de la durée du traitement
duration=$((end_time - start_time))
echo "Durée du traitement: $duration secondes"

# Génération des graphiques (bonus)
if [[ "$type_station" == "lv" && "$type_consommateur" == "all" ]]; then
    if ! command -v gnuplot &> /dev/null; then
        echo "Erreur: gnuplot n'est pas installé. Ignorer la génération des graphiques."
    else
        echo "Génération des graphiques..."
        gnuplot -e "set terminal png; set output 'graphs/lv_all_minmax.png'; plot '$temp_output' using 2:xtic(1) with bars"
    fi
fi

echo "Traitement terminé."
