#include <stdio.h>
#include <string.h>

#include "lexer.h"

int yylex();
extern char *yytext;
extern int yyleng;
extern FILE *yyin;
extern int yylineno;

void *yylval;

int main(int argc, char *argv[]) {
FILE *input, *output;

int token;

struct character *character;

    /* Figure out whether we're using stdin/stdout or file in/file out. */
    if (argc < 2 || !strcmp("-", argv[1])) {
        input = stdin;
    } else {
        input = fopen(argv[1], "r");
    }

    if (argc < 3 || !strcmp("-", argv[2])) {
        output = stdout;
    } else {
        output = fopen(argv[2], "w");
    }

/* Tell lex where to get input. */
yyin = input;

/* Begin scanning. */
token = yylex();
while (0 != token) {
    character = (struct character *) yylval;
    if (token == CHAR_CONSTANT)
    printf("token: CHARACTER CONSTANT value: %c\n", character->c);

      token = yylex();
    }

    return 0;
}
