#include <sys/types.h>

#define YYSTYPE void *

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
    E_INVALID_CHAR = -7
};

enum token {
    UNRECOGNIZED = 1,
    CHAR_CONSTANT = 2,
    STRING_CONSTANT = 3,
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
    WHILE
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


int convert_single_escape(char c);
int convert_octal_escape(char *seq, int n_digits);
static inline int isodigit(const char c);
struct Character *create_character(char c);
void handle_error(enum lexer_error e, char *data, int line);
void emalloc(void **ptr, size_t n);
