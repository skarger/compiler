#include <sys/types.h>

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

enum integer_type {
    OVERFLOW,
    SIGNED_SHORT,
    UNSIGNED_SHORT,
    SIGNED_INT,
    UNSIGNED_INT,
    SIGNED_LONG,
    UNSIGNED_LONG
};

enum token {
    UNRECOGNIZED = 1,
    CHAR_CONSTANT,
    STRING_CONSTANT,
    NUMBER_CONSTANT,
    IDENTIFIER,
    BREAK,
    CHAR,
    CONTINUE,
    DO,
    ELSE,
    FOR,
    GOTO,
    IF,
    INT,
    LONG,
    RETURN,
    SIGNED,
    SHORT,
    UNSIGNED,
    VOID,
    WHILE,
    LOGICAL_NOT,
    REMAINDER,
    BITWISE_XOR,
    AMPERSAND,
    ASTERISK,
    MINUS,
    PLUS,
    ASSIGN,
    BITWISE_NOT,
    BITWISE_OR,
    LESS_THAN,
    GREATER_THAN,
    DIVIDE,
    TERNARY_CONDITIONAL,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    SEMICOLON,
    COLON,
    ADD_ASSIGN,
    SUBTRACT_ASSIGN,
    MULTIPLY_ASSIGN,
    DIVIDE_ASSIGN,
    REMAINDER_ASSIGN,
    BITWISE_LSHIFT_ASSIGN,
    BITWISE_RSHIFT_ASSIGN,
    BITWISE_AND_ASSIGN,
    BITWISE_XOR_ASSSIGN,
    BITWISE_OR_ASSIGN,
    INCREMENT,
    DECREMENT,
    BITWISE_LSHIFT,
    BITWISE_RSHIFT,
    LESS_THAN_EQUAL,
    GREATER_THAN_EQUAL,
    EQUAL,
    NOT_EQUAL,
    LOGICAL_AND,
    LOGICAL_OR
};

struct Character {
    char c;
};

struct String {
    char *str; /* pointer to the beginning of the string */
    char *current; /* pointer to an arbitrary character in str, used for construction */
    int length; /* the length of the string */
    enum Boolean valid; /* flag for whether str is valid by the standards of CSCI-E95 C */
};

struct Number {
    unsigned long value;
    enum integer_type type;
};

struct Character *create_character(char c);
struct String *create_string(int len);
struct Number *create_number(char *digit_str);
