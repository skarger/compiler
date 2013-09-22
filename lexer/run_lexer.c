
/*
 * This file heavily inspired by class provided scanner_main.c
 * http://sites.fas.harvard.edu/~libe295/fall2013/willenson/asst1/scanner_main.c
 *  main
 *    The entry point for assignment one. reads input from either stdin or a
 *    file specified on the command line. Writes output to stdout or a file
 *    specified on the command line. After each call to yylex(), decodes the
 *    name of the returned token, or prints an error if no token was found, or
 *    exits if the input stream has ended. Prints the current line number, name
 *    of the token, and other information depending on the type of the returned
 *    token.
 *
 *  Arguments:
 *    First argument specifies the input file. if it is not provided or is equal
 *      to "-", input file is assumed to be stdin.
 *    Second argument specifies the output file. if it is not provided or is 
 *      equal to "-", output file is assumed to be stdout.
 *
 *  Exit values:
 *    0 for normal
 *    1 for out of memory
 *
 */
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

struct Number *number;
struct Character *character;
struct String *string;

char *get_token_name(int token);
char *get_integer_type(enum integer_type type);

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
        case IDENTIFIER:
            string = (struct String *) yylval;
            printf("identifier: %s\n", string->str);
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
            printf("reserved word: %s\n", get_token_name(token));
            break;
        default:
            break;
    }

    if (token == NUMBER_CONSTANT) {
        number = (struct Number *) yylval;
        printf("token: NUMBER CONSTANT type: %s value: %u\n",
                    get_integer_type(number->type), number->value);
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

#include <string.h>


/*
 * get_token_name 
 *   Get the name of a token returned by yylex()
 *
 * Parameters:
 *   token - int - the token returned by yylex()
 * 
 * Return: A pointer to the zero-terminated the token name.
 * Side effects: none
 *
 */
char *get_token_name(int token) {
    switch (token) {
    #define CASE_FOR(token) case token: return #token
        CASE_FOR(IDENTIFIER);
        CASE_FOR(CHAR);
        CASE_FOR(CONTINUE);
        CASE_FOR(DO);
        CASE_FOR(ELSE);
        CASE_FOR(FOR);
        CASE_FOR(GOTO);
        CASE_FOR(IF);
        CASE_FOR(INT);
        CASE_FOR(LONG);
        CASE_FOR(RETURN);
        CASE_FOR(SHORT);
        CASE_FOR(SIGNED);
        CASE_FOR(UNSIGNED);
        CASE_FOR(BREAK);
        CASE_FOR(VOID);
        CASE_FOR(WHILE);
    #undef CASE_FOR
        default: return "";
  }
}

char *get_integer_type(enum integer_type type) {
    switch (type) {
    #define CASE_FOR(token) case token: return #token
        CASE_FOR(OVERFLOW);
        CASE_FOR(SIGNED_SHORT);
        CASE_FOR(UNSIGNED_SHORT);
        CASE_FOR(SIGNED_INT);
        CASE_FOR(UNSIGNED_INT);
        CASE_FOR(SIGNED_LONG);
        CASE_FOR(UNSIGNED_LONG);
    #undef CASE_FOR
        default: return "";
    }
}
