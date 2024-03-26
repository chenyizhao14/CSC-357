CC=gcc

CFLAGS=-g

LDFLAGS=-lm

HEADER = create_archive.h list_archive.h extract_archive.h special_int.h

MYTAR = mytar.c create_archive.c list_archive.c extract_archive.c special_int.c

mytar: $(MYTAR) $(HEADER)
	$(CC) $(CLFAGS) $(LDFLAGS) -o mytar $(MYTAR)
