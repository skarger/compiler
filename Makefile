CC = gcc
LEX = flex
YACC = yacc
CFLAGS += -pedantic -Wall -Wextra
LDLIBS += -lfl -ly
YFLAGS += -d

VPATH = src
EXECS = lexer parser

all : $(EXECS)


clean :
	rm -f $(EXECS) *.o lex.yy.c y.tab.c

lex.yy.o : lex.yy.c
	$(CC) -c lex.yy.c

lex.yy.c : lexer/lexer.lex src/include/lexer.h
	$(LEX) $(LFLAGS) -o $@ $<

lexer.o : src/lexer/lexer.c lex.yy.o src/include/lexer.h
	$(CC) -c src/lexer/lexer.c lex.yy.c src/include/lexer.h

lexer : lexer.o
	$(CC) lexer.o lex.yy.o -o $@

y.tab.c : src/parser/parser.y lex.yy.c
	$(YACC) $(YFLAGS) -o $@ $<

parser : y.tab.c
	$(CC) y.tab.c -o $@

