# Variabili
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lz
SRC = source/main.c source/utils.c source/fsp.c
OBJ = $(patsubst source/%.c, build/%.o, $(SRC))
TARGET = build/fsplitter

# Regole
all: $(TARGET)

# Crea l'eseguibile dalla lista di oggetti
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

# Regola per compilare i .c in build/*.o
build/%.o: source/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Crea la cartella build se non esiste
build:
	mkdir -p build

# Pulisce tutto
clean:
	rm -rf build

.PHONY: all clean build
