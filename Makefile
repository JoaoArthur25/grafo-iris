CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -O2

grafo_iris: main.c grafo.c grafo.h
	$(CC) $(CFLAGS) -o grafo_iris main.c grafo.c -lm
