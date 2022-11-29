CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -g

.PHONY: clean

test_program: test_program.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	-rm test_program
