CC=gcc

CFLAGS=-MMD -Wall -Wextra -pedantic -std=c23

all: debug

debug: CFLAGS += -g
debug: main

release: CFLAGS += -O2 -DNDEBUG
release: main

main:
	${CC} ${CFLAGS} main.c -o pplss
