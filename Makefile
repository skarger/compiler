CC = gcc
CPP = g++
LEX = flex
YACC = yacc
CFLAGS += -pedantic -Wall -Wextra
CXXFLAGS += -g -Wall -Wextra
LDLIBS += -lfl -ly
YFLAGS += -d

VPATH = src
GTEST_DIR = ./test/gtest-1.7.0


TESTS = libgtest.a symbol-test gtest-symbol
EXECS = lexer parser
SRCS = y.tab.c lex.yy.c src/lexer/lexer.c src/utilities/utilities.c \
src/symbol/symbol.c src/symbol/symbol-utils.c src/symbol/symbol-test.c


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

parser : y.tab.o utilities.o traverse.o symbol-utils.o
	$(CC) y.tab.o utilities.o traverse.o symbol-utils.o -o $@

symbol-utils.o : src/symbol/symbol-utils.c
	$(CC) -c src/symbol/symbol-utils.c

traverse.o : src/symbol/traverse.c
	$(CC) -c src/symbol/traverse.c


# tests
libtest.a :
	$(CPP) -isystem ${GTEST_DIR}/include -I${GTEST_DIR} \
-pthread -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o

gtest-symbol : libtest.a ./test/symbol/gtest-symbol.cpp
	$(CPP) -isystem ${GTEST_DIR}/include -pthread \
./test/symbol/gtest-symbol.cpp libgtest.a -o $@

symbol-test.o : src/symbol/symbol-test.c
	$(CC) -c src/symbol/symbol-test.c

symbol-test : symbol-utils.o symbol-test.o utilities.o
	$(CC) symbol-utils.o symbol-test.o utilities.o -o $@

test-symbol-output : test/symbol/test-symbol-output parser
	test/symbol/test-symbol-output
