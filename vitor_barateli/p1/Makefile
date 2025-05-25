CC = gcc
CFLAGS = -Wall -O2

TARGETS = compilador assembler executor

all: $(TARGETS)

compilador: compilador.c
	$(CC) $(CFLAGS) -o compilador compilador.c

assembler: assembler.c
	$(CC) $(CFLAGS) -o assembler assembler.c

executor: executor.c
	$(CC) $(CFLAGS) -o executor executor.c

clean:
	rm -f $(TARGETS)
