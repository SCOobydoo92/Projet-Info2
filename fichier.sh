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
identifiant_centrale=$4

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
    echo "Erreur: seules les entreprises sont connectées aux stations HV-B et HV-A"[^1^][1]
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
start_time=$(date +%s)
./codeC/cwire "$chemin_fichier_csv" "$type_station" "$type_consommateur" "$identifiant_centrale"
end_time=$(date +%s)

# Calcul et affichage de la durée du traitement
duration=$((end_time - start_time))
echo "Durée du traitement: $duration secondes"

# Génération des graphiques (bonus)
if [[ "$type_station" == "lv" && "$type_consommateur" == "all" ]]; then
    echo "Génération des graphiques..."
    gnuplot -e "set terminal png; set output 'graphs/lv_all_minmax.png'; plot 'tmp/lv_all_minmax.csv' using 2:xtic(1) with bars"
fi

echo "Traitement terminé."
