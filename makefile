# Nom de l'exécutable final
TARGET = cwire

# Compilateur utilisé
CC = gcc

# Options de compilation
CFLAGS = -Wall -Wextra -g

# Fichiers sources
SRC = main.c

# Fichiers objets générés
OBJ = $(SRC:.c=.o)

# Règle par défaut : compilation complète
all: $(TARGET)

# Règle pour générer l'exécutable
$(TARGET): $(OBJ)
	@echo "Génération de l'exécutable $(TARGET)..."
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)
	@echo "Compilation terminée."

# Règle pour compiler les fichiers .c en .o
%.o: %.c
	@echo "Compilation de $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers intermédiaires
clean:
	@echo "Suppression des fichiers objets et de l'exécutable..."
	rm -f $(OBJ) $(TARGET)

# Nettoyage complet
distclean: clean
	@echo "Suppression complète des fichiers temporaires."
	rm -rf tmp/* graphs/*

# Aide pour l'utilisation du Makefile
help:
	@echo "Utilisation :"
	@echo "  make         : Compiler le programme"
	@echo "  make clean   : Nettoyer les fichiers objets et l'exécutable"
	@echo "  make distclean : Nettoyer tous les fichiers temporaires"
	@echo "  make help    : Afficher cette aide"
