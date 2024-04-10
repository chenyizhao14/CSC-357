CC = gcc

CFLAGS = -g -Wall

mush2: mush2.o
	$(CC) $(CFLAGS) -L ~pn-cs357/Given/Mush/lib64 -o mush2 mush2.o -lmush

mush2.o: mush2.c
	$(CC) $(CFLAGS) -c -I ~pn-cs357/Given/Mush/include -o mush2.o mush2.c
