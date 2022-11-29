CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -Wpedantic
PROGS=test_program bf

.PHONY: all clean

all: $(PROGS)

test_program: test_program.c bf.h
	$(CC) $(CFLAGS) -o $@ $<

bf: bf.c bf.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	-rm $(PROGS)
