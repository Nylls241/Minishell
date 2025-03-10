CC = gcc
CFLAGS = -Wall -Wextra

all: mishell

mishell: mishell.c
	$(CC) $(CFLAGS) -o mishell mishell.c

clean:
	rm -f mishell