CC = gcc
LEX = flex
YACC = yacc
CFLAGS += -pedantic -Wall -Wextra
LDLIBS += -lfl -ly
YFLAGS += -d

VPATH = src

EXECS = lexer parser symbol
SRCS = y.tab.c lex.yy.c src/lexer/lexer.c src/utilities/utilities.c src/symbol/symbol.c

all : $(EXECS)



clean :
	rm -f $(EXECS) *.o lex.yy.c y.tab.c y.tab.h

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
parser : y.tab.o utilities.o
	$(CC) y.tab.o utilities.o -o $@

symbol.o :src/symbol/symbol-utils.c
	$(CC) -c src/symbol/symbol-utils.c -o symbol.o

symbol : symbol.o
	$(CC) symbol.o -o $@
