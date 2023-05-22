CC=gcc

CFLAGS=-Wall -ansi -Werror -pedantic -g

LDFLAGS=-lm

all: extract create 

extract: extract_archive.c extract_archive.h create_archive.h
	$(CC) $(CFLAGS) -o extract extract_archive.c create_archive.h $(LDFLAGS)

create: create_archive.c create_archive.h
	$(CC) $(CFLAGS) -o create create_archive.c create_archive.h $(LDFLAGS)
