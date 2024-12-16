TARGET = cwire

CC = gcc

CFLAGS = -Wall -Wextra -g

SRC = main.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)


distclean: clean
	rm -f tmp/* graphs/*
