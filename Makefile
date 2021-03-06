CC = gcc
CPP = g++
LEX = flex
YACC = yacc
CFLAGS += -g -pedantic -Wall -Wextra
CXXFLAGS += -g -Wall -Wextra
LDLIBS += -lfl -ly
YFLAGS += -d
GTEST_DIR = test/gtest-1.7.0


VPATH = src


TESTS = libgtest.a test-ir test-symbol-utils test/symbol/st-output
EXECS = lexer-main parser-main symbol-main ir-main mips-main
SRCS = y.tab.c lex.yy.c src/lexer/lexer-main.c src/utilities/utilities.c \
src/parser/parser-main.c src/cmpl/cmpl.c \
src/symbol/symbol-utils.c test/symbol/test-symbol-utils.c \
src/symbol/symbol-main.c src/symbol/scope-fsm.c \
src/ir/ir-main.c src/ir/ir-utils.c \
src/mips/mips-main.c src/mips/mips-utils.c \


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

lexer-main.o : lex.yy.o
	$(CC) -c src/lexer/lexer-main.c lex.yy.c

lexer-main : lexer-main.o
	$(CC) lexer-main.o lex.yy.o -o $@

y.tab.c : src/parser/parser.y lex.yy.c
	$(YACC) $(YFLAGS) -o $@ $<

y.tab.o :
	$(CC) -c y.tab.c

parser-main.o : src/parser/parser-main.c
	$(CC) -c src/parser/parser-main.c -o $@

parser-main : parser-main.o y.tab.o utilities.o \
symbol-collection.o symbol-utils.o scope-fsm.o ir-utils.o mips-utils.o
	$(CC) parser-main.o y.tab.o utilities.o \
symbol-collection.o symbol-utils.o scope-fsm.o ir-utils.o mips-utils.o -o $@

symbol-main.o : src/symbol/symbol-main.c
	$(CC) -c src/symbol/symbol-main.c -o $@

symbol-main : symbol-main.o y.tab.o utilities.o \
symbol-collection.o symbol-utils.o scope-fsm.o ir-utils.o mips-utils.o
	$(CC) symbol-main.o y.tab.o utilities.o \
symbol-collection.o symbol-utils.o scope-fsm.o ir-utils.o mips-utils.o -o $@

symbol-utils.o : src/symbol/symbol-utils.c
	$(CC) -c src/symbol/symbol-utils.c

symbol-collection.o : src/symbol/symbol-collection.c
	$(CC) -c src/symbol/symbol-collection.c

cmpl.o : src/cmpl/cmpl.c
	$(CC) -c src/cmpl/cmpl.c

scope-fsm.o : src/symbol/scope-fsm.c
	$(CC) -c src/symbol/scope-fsm.c

ir-main : ir-main.o mips-utils.o ir-utils.o y.tab.o \
scope-fsm.o symbol-collection.o symbol-utils.o utilities.o
	$(CC) ir-main.o mips-utils.o ir-utils.o y.tab.o \
scope-fsm.o symbol-collection.o symbol-utils.o utilities.o -o $@

ir-main.o : src/ir/ir-main.c
	$(CC) -c src/ir/ir-main.c

ir-utils.o : src/ir/ir-utils.c
	$(CC) -c src/ir/ir-utils.c

mips-main : mips-main.o mips-utils.o ir-utils.o y.tab.o \
scope-fsm.o symbol-collection.o symbol-utils.o utilities.o
	$(CC) mips-main.o mips-utils.o ir-utils.o y.tab.o \
scope-fsm.o symbol-collection.o symbol-utils.o utilities.o -o $@

mips-main.o : src/mips/mips-main.c
	$(CC) -c src/mips/mips-main.c

mips-utils.o : src/mips/mips-utils.c
	$(CC) -c src/mips/mips-utils.c

# tests
test-parser-output : parser-main
	./test/parser/test-parser-output 2>/dev/null

test-parser-errors : parser-main
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

test-mips: mips-main
	./test/mips/test-mips

test-ir : test/ir/test-ir.cpp libgtest.a \
ir-utils.o mips-utils.o y.tab.o cmpl.o \
scope-fsm.o symbol-collection.o symbol-utils.o \
utilities.o
	g++ -isystem ${GTEST_DIR}/include -pthread test/ir/test-ir.cpp libgtest.a \
ir-utils.o mips-utils.o y.tab.o cmpl.o \
scope-fsm.o symbol-collection.o symbol-utils.o \
utilities.o -o $@
	./test-ir

libgtest.a : gtest-all.o
	ar -rv libgtest.a gtest-all.o

gtest-all.o :
	g++ -isystem ${GTEST_DIR}/include -I${GTEST_DIR} \
-pthread -c ${GTEST_DIR}/src/gtest-all.cc

