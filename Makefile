CC = gcc
LEX = flex
CFLAGS += -pedantic -Wall -Wextra
LDLIBS += -lfl

VPATH = src
EXECS = lexer parser

all : $(EXECS)


clean :
	rm -f $(EXECS) *.o lex.yy.c

lex.yy.o : lex.yy.c
	$(CC) -c lex.yy.c

lex.yy.c : lexer/lexer.lex src/include/lexer.h
	$(LEX) $(LFLAGS) -o $@ $<

lexer.o : src/lexer/lexer.c lex.yy.o src/include/lexer.h
	$(CC) -c src/lexer/lexer.c lex.yy.c src/include/lexer.h

lexer : lexer.o
	$(CC) lexer.o lex.yy.o -o $@

parser.o : src/parser/parser.c
	$(CC) -c src/parser/parser.c

parser : parser.o
	$(CC) parser.o -o $@

