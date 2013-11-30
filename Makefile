CC = gcc
CPP = g++
LEX = flex
YACC = yacc
CFLAGS += -pedantic -Wall -Wextra
CXXFLAGS += -g -Wall -Wextra
LDLIBS += -lfl -ly
YFLAGS += -d

VPATH = src


TESTS = test-symbol-utils test/symbol/st-output
EXECS = lexer parser-main symbol-main
SRCS = y.tab.c lex.yy.c src/lexer/lexer.c src/utilities/utilities.c \
src/parser/parser-main.c src/symbol/traverse.c \
src/symbol/symbol-utils.c test/symbol/test-symbol-utils.c \
src/symbol/symbol-main.c src/symbol/scope-fsm.c


all : $(EXECS)

clean :
	rm -f $(TESTS) $(EXECS) *.o lex.yy.c y.tab.c y.tab.h

# autmatically pull in dependencies on included header files
# copied from http://stackoverflow.com/a/2394668/1424966
.depend: $(SRCS)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ >>./.depend
include .depend

utilities.o :
	$(CC) -c src/utilities/utilities.c

lex.yy.o :
	$(CC) -c lex.yy.c

lex.yy.c : lexer/lexer.lex
	$(LEX) $(LFLAGS) -o $@ $<

lexer.o : lex.yy.o
	$(CC) -c src/lexer/lexer.c lex.yy.c

lexer : lexer.o
	$(CC) lexer.o lex.yy.o -o $@

y.tab.c : src/parser/parser.y lex.yy.c
	$(YACC) $(YFLAGS) -o $@ $<

y.tab.o :
	$(CC) -c y.tab.c

parser-main.o : src/parser/parser-main.c
	$(CC) -c src/parser/parser-main.c -o $@

parser-main : parser-main.o y.tab.o utilities.o traverse.o \
symbol-utils.o scope-fsm.o
	$(CC) parser-main.o y.tab.o utilities.o traverse.o \
symbol-utils.o scope-fsm.o -o $@

symbol-main.o : src/symbol/symbol-main.c
	$(CC) -c src/symbol/symbol-main.c -o $@

symbol-main : symbol-main.o y.tab.o utilities.o traverse.o \
symbol-utils.o scope-fsm.o
	$(CC) symbol-main.o y.tab.o utilities.o traverse.o \
symbol-utils.o scope-fsm.o -o $@

symbol-utils.o : src/symbol/symbol-utils.c
	$(CC) -c src/symbol/symbol-utils.c

traverse.o : src/symbol/traverse.c
	$(CC) -c src/symbol/traverse.c

scope-fsm.o : src/symbol/scope-fsm.c
	$(CC) -c src/symbol/scope-fsm.c

# tests
test-parser-output : parser-main
	./test/parser/test-parser-output

test-symbol : test-symbol-utils test-symtab-output
	./test/symbol/test-symtab-output 2>/dev/null
	./test-symbol-utils

test-symtab-errors : symbol-main
	./test/symbol/test-symtab-output 

test-symbol-utils.o : test/symbol/test-symbol-utils.c
	$(CC) -c test/symbol/test-symbol-utils.c

test-symbol-utils : symbol-utils.o test-symbol-utils.o utilities.o scope-fsm.o
	$(CC) symbol-utils.o test-symbol-utils.o utilities.o scope-fsm.o -o $@	

test-symtab-output : symbol-main


