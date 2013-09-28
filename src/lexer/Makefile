CC = gcc
LEX = flex
CFLAGS += -pedantic -Wall -Wextra
LDLIBS += -lfl


EXECS = lexer

all : $(EXECS)


clean :
	rm -f $(EXECS) *.o lex.yy.c

lex.yy.o : lex.yy.c
	$(CC) -c lex.yy.c

lex.yy.c : lexer.lex lexer.h
	$(LEX) $(LFLAGS) -o $@ $<

lexer.o : lexer.c lex.yy.o lexer.h
	$(CC) -c lexer.c lex.yy.c lexer.h

lexer : lexer.o
	$(CC) lexer.o lex.yy.o -o $@