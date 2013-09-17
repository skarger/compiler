CC = gcc
LEX = flex
CFLAGS += -pedantic -Wall -Wextra
LDLIBS += -lfl


EXECS = lexer

all : $(EXECS)

clean :
	rm -f $(EXECS) *.o lex.yy.c

lexer : lex.yy.o lexer.h 
	$(CC) $(CFLAGS) lex.yy.o -o $@ $(LDLIBS)

lex.yy.o : lex.yy.c
	$(CC) -c lex.yy.c

lex.yy.c : lexer.lex lexer.h
	$(LEX) $(LFLAGS) -o $@ $<