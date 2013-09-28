/*
 * This file heavily inspired by class provided scanner_main.c
 * http://sites.fas.harvard.edu/~libe295/fall2013/willenson/asst1/scanner_main.c
 * The description from that file remains true for this version:
 *
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
 */
#include <stdio.h>
#include <string.h>

#include "../include/lexer.h"

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
    /* 
     * Print the line number. Use printf formatting and tabs to keep columns 
     * lined up.
     */
    fprintf(output, "line = %-5d\t", yylineno);

    /* 
     * Print the scanned text. Try to use formatting but give up instead of 
     * truncating if the text is too long.
     * Do not print scanned string text. TODO: change that?
     */
    if (token == STRING_CONSTANT) {
        fprintf(output, "    %-20s\t", "");
    } else {
        fprintf(output, (yyleng < 20 ? "text = %-20s\t" : "text = %s\t"), yytext);
    }

    if (token != UNRECOGNIZED) {
        /* Look up and print the token's name. */
        fprintf(output, "token = %-15s\t", get_token_name(token));

        switch (token) {
            case IDENTIFIER:
                string = (struct String *) yylval;
                fprintf(output, "    id = %s\n", string->str);
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
                fprintf(output, "    rsvwd = %-20s\n", get_token_name(token));
                break;
            case LOGICAL_NOT:
            case REMAINDER:
            case BITWISE_XOR:
            case AMPERSAND:
            case ASTERISK:
            case MINUS:
            case PLUS:
            case ASSIGN:
            case BITWISE_NOT:
            case BITWISE_OR:
            case LESS_THAN:
            case GREATER_THAN:
            case DIVIDE:
            case TERNARY_CONDITIONAL:
                fprintf(output, "    op = %-20s\n", get_token_name(token));
                break;
            case LEFT_PAREN:
            case RIGHT_PAREN:
            case LEFT_BRACKET:
            case RIGHT_BRACKET:
            case LEFT_BRACE:
            case RIGHT_BRACE:
            case COMMA:
            case SEMICOLON:
            case COLON:
                fprintf(output, "    sep = %-20s\n", get_token_name(token));
                break;
            case ADD_ASSIGN:
            case SUBTRACT_ASSIGN:
            case MULTIPLY_ASSIGN:
            case DIVIDE_ASSIGN:
            case REMAINDER_ASSIGN:
            case BITWISE_LSHIFT_ASSIGN:
            case BITWISE_RSHIFT_ASSIGN:
            case BITWISE_AND_ASSIGN:
            case BITWISE_XOR_ASSSIGN:
            case BITWISE_OR_ASSIGN:
                fprintf(output, "    cmp asgn op = %-20s\n", get_token_name(token));
                break;
            case INCREMENT:
            case DECREMENT:
            case BITWISE_LSHIFT:
            case BITWISE_RSHIFT:
            case LESS_THAN_EQUAL:
            case GREATER_THAN_EQUAL:
            case EQUAL:
            case NOT_EQUAL:
            case LOGICAL_AND:
            case LOGICAL_OR:
                fprintf(output, "    cmp op = %-20s\n", get_token_name(token));
                break;
            case NUMBER_CONSTANT:
                number = (struct Number *) yylval;
                fprintf(output, "\ttype = %20s\tvalue = %-10lu\n", 
                        get_integer_type(number->type),
                        number->value);
                break;
            case CHAR_CONSTANT:
                character = (struct Character *) yylval;
                fprintf(output, "\tvalue: %c\n", character->c);
                break;
            case STRING_CONSTANT:
                string = (struct String *) yylval;
                fprintf(output, "\tvalue: %s\n", string->str);
                break;
            default:
                break;
        }
    } else {
        /* unrecognized input */
        fputs("error = SCANNING ERROR\n", output);
    }
    token = yylex();
}

    /* Scanning complete. */
    if (output != stdout) {
    fclose(output);
    }
    if (input != stdin) {
    fclose(input);
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
        CASE_FOR(CHAR_CONSTANT);
        CASE_FOR(STRING_CONSTANT);
        CASE_FOR(NUMBER_CONSTANT);
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
        CASE_FOR(LOGICAL_NOT);
        CASE_FOR(REMAINDER);
        CASE_FOR(BITWISE_XOR);
        CASE_FOR(AMPERSAND);
        CASE_FOR(ASTERISK);
        CASE_FOR(MINUS);
        CASE_FOR(PLUS);
        CASE_FOR(ASSIGN);
        CASE_FOR(BITWISE_NOT);
        CASE_FOR(BITWISE_OR);
        CASE_FOR(LESS_THAN);
        CASE_FOR(GREATER_THAN);
        CASE_FOR(DIVIDE);
        CASE_FOR(TERNARY_CONDITIONAL);
        CASE_FOR(LEFT_PAREN);
        CASE_FOR(RIGHT_PAREN);
        CASE_FOR(LEFT_BRACKET);
        CASE_FOR(RIGHT_BRACKET);
        CASE_FOR(LEFT_BRACE);
        CASE_FOR(RIGHT_BRACE);
        CASE_FOR(COMMA);
        CASE_FOR(SEMICOLON);
        CASE_FOR(COLON);
        CASE_FOR(ADD_ASSIGN);
        CASE_FOR(SUBTRACT_ASSIGN);
        CASE_FOR(MULTIPLY_ASSIGN);
        CASE_FOR(DIVIDE_ASSIGN);
        CASE_FOR(REMAINDER_ASSIGN);
        CASE_FOR(BITWISE_LSHIFT_ASSIGN);
        CASE_FOR(BITWISE_RSHIFT_ASSIGN);
        CASE_FOR(BITWISE_AND_ASSIGN);
        CASE_FOR(BITWISE_XOR_ASSSIGN);
        CASE_FOR(BITWISE_OR_ASSIGN);
        CASE_FOR(INCREMENT);
        CASE_FOR(DECREMENT);
        CASE_FOR(BITWISE_LSHIFT);
        CASE_FOR(BITWISE_RSHIFT);
        CASE_FOR(LESS_THAN_EQUAL);
        CASE_FOR(GREATER_THAN_EQUAL);
        CASE_FOR(EQUAL);
        CASE_FOR(NOT_EQUAL);
        CASE_FOR(LOGICAL_AND);
        CASE_FOR(LOGICAL_OR);
    #undef CASE_FOR
        default: return "";
  }
}

/*
 * get_integer_type
 *   Get the string name of an integer type returned by yylex()
 *
 * Parameters:
 *   type - enum integer_type - the type of an integral constant found by yylex()
 * 
 * Return: A pointer to the zero-terminated the type name.
 * Side effects: none
 *
 */
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
