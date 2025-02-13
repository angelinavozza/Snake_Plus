CC = gcc
CFLAGS = -g -Wall
LIBS = -lncurses
TARGETS = snake_plus

all: $(TARGETS)

snake_plus: snake_plus.c
	$(CC) $(CFLAGS) $(LIBS) snake_plus.c -o $@

clean:
		rm -f snake_plus
		rm -rf *.dSYM
