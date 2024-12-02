#!/bin/bash

# Définir les noms de fichier
SOURCE_FILE="avl_tree.c"    # Fichier source C
OUTPUT_FILE="avl_tree"      # Fichier exécutable

# Vérifier si le fichier source existe
if [ ! -f "$SOURCE_FILE" ]; then
    echo "Erreur : le fichier source '$SOURCE_FILE' n'existe pas."
    exit 1
fi

# Compiler le programme
echo "Compilation de $SOURCE_FILE..."
gcc -o "$OUTPUT_FILE" "$SOURCE_FILE" -Wall

# Vérifier si la compilation a réussi
if [ $? -ne 0 ]; then
    echo "Erreur de compilation."
    exit 1
fi

# Si la compilation est réussie, exécuter le programme
echo "Exécution du programme..."
./"$OUTPUT_FILE"

# Vérifier si l'exécution a réussi
if [ $? -ne 0 ]; then
    echo "Erreur lors de l'exécution du programme."
    exit 1
fi

echo "Exécution terminée avec succès."
