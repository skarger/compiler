CC = gcc -pedantic -Wall
LEX = flex
CFLAGS += -pedantic -Wall -Wextra
LDLIBS += -lfl


EXECS = lexer

all : $(EXECS)

lex.yy.c : chars.lex
	$(LEX) $(LFLAGS) -o $@ $<

lexer : lexer.h chars.lex lex.yy.c
	$(CC) lex.yy.c $(LDLIBS)
	