PRJ=ifj2019
#
PROGS=$(PRJ)
FILES=main.c scanner.c parser.c expression.c symtable.c generator.c
CC=gcc
CFLAGS=-Wall -W -Wextra -g -std=c99 -pedantic


all: $(PROGS)

$(PRJ): $(FILES)
	$(CC) $(CFLAGS) -o $@ $(FILES) -lm

clean:
	rm -f *.o *.out $(PROGS)

test: all
	chmod +x run_tests.sh
	./run_tests.sh