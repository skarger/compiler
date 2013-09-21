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

struct Character *character;
struct String *string;

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
    switch (token) {
        case ID:
            printf("identifier\n");
            break;
        case BREAK:
        case CHAR:
        case CONTINUE:
        case DO:
        case ELSE:
        case FOR:
        case GOTO:
        case IF:
        case INT:
        case LONG:
        case RETURN:
        case SIGNED:
        case SHORT:
        case UNSIGNED:
        case VOID:
        case WHILE:
            printf("reserved word\n");
            break;
        default:
            break;
    }

    if (token == CHAR_CONSTANT) {
        character = (struct Character *) yylval;
        printf("token: CHARACTER CONSTANT value: %c\n", character->c);
    }

    if (token == STRING_CONSTANT) {
        string = (struct String *) yylval;
        printf("token: STRING CONSTANT value: %s\n", string->str);
    }

    if (token == UNRECOGNIZED) {
        ;
    }

      token = yylex();
}

    return 0;
}
