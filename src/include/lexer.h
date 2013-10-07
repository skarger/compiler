/*
 * Definitions, enumerations, structures and function prototypes
 * used in lexer.lex and lexer.c.
 * Those files are the Lex file and corresponding standalone runner for the
 * CSCI-E95 C language scanner.
 */
#include <sys/types.h>


#ifndef LEXER_H
#define LEXER_H

#define YYSTYPE void *

#define CHAR_BYTES 1
#define SHORT_BYTES 2
#define INT_BYTES 4
#define LONG_BYTES 4

enum Boolean {
    FALSE = 0,
    TRUE = 1
};

enum lexer_error {
    E_SUCCESS,
    E_ESCAPE_SEQ = -1,
    E_NOT_OCTAL = -2,
    E_MALLOC = -3,
    E_NEWLINE = -4,
    E_INVALID_STRING = -5,
    E_INVALID_ID = -6,
    E_INVALID_CHAR = -7,
    E_EMPTY_CHAR = -8,
    E_OCTAL = -9,
    E_FLOAT = -10,
    E_INTEGER_OVERFLOW = -11
};

enum data_type {
    SIGNED_CHAR,
    UNSIGNED_CHAR,
    SIGNED_SHORT,
    UNSIGNED_SHORT,
    SIGNED_INT,
    UNSIGNED_INT,
    SIGNED_LONG,
    UNSIGNED_LONG,
    OVERFLOW
};

struct Character {
    char c;
};

struct String {
    char *str; /* pointer to the beginning of the string */
    char *current; /* pointer to an arbitrary character in str used for construction */
    int length; /* the length of the string */
    enum Boolean valid; /* flag for whether str is valid by the standards of CSCI-E95 C */
};

struct Number {
    unsigned long value;
    enum data_type type;
};

struct Character *create_character(char c);
struct String *create_string(int len);
struct Number *create_number(char *digit_str);

char *get_token_name(int token);

#endif /* LEXER_H */
