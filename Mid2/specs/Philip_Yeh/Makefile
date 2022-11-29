CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic

.PHONY: all clean

all: test_program

caesar_cipher: caesar_cipher.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	-rm test_program

