CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lcurl

all:
	$(CC) $(CFLAGS) src/dget.c -o dget $(LDFLAGS)

clean:
	rm -f dget
