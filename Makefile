CC=gcc
CFLAGS=-O2 -Wall
LIBS=-lcurl

all:
	$(CC) src/dget.c -o dget $(LIBS)

clean:
	rm -f dget
