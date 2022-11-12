CC=gcc
CFLAGS=-std=c11 -Wextra -O3

all:
	$(CC) -o raycaster raycaster.c -lSDL2 -lm
