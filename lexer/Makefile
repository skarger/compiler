CC = gcc
LEX = flex
CFLAGS += -pedantic -Wall -Wextra
LDLIBS += -lfl


EXECS = lexer

TESTS = run_lexer

all : $(EXECS)

test : $(TESTS)
	./run_lexer

clean :
	rm -f $(EXECS) *.o lex.yy.c

lexer : lex.yy.o lexer.h 
	$(CC) $(CFLAGS) lex.yy.o -o $@ $(LDLIBS)

lex.yy.o : lex.yy.c
	$(CC) -c lex.yy.c

lex.yy.c : lexer.lex lexer.h
	$(LEX) $(LFLAGS) -o $@ $<

run_lexer.o : run_lexer.c lex.yy.o lexer.h
	$(CC) -c run_lexer.c lex.yy.c lexer.h

run_lexer : run_lexer.o
	$(CC) run_lexer.o lex.yy.o -o $@