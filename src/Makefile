PRJ=scanner
#
PROGS=$(PRJ)
FILES=main_scanner.c scanner.c
PARSER_FILES=main_parser.c scanner.c parser.c semantic.c symtable.c
CC=gcc			
CFLAGS=-Wall -W -Wextra -g -std=c99 -pedantic


all: $(PROGS) parser

$(PRJ): $(FILES)
	$(CC) $(CFLAGS) -o $@ $(FILES) -lm

parser: $(PARSER_FILES)
	$(CC) $(CFLAGS) -o $@ $(PARSER_FILES) -lm

clean:
	rm -f *.o *.out $(PROGS) parser semantic