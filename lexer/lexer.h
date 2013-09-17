#define YYSTYPE void *

enum lexer_error {
    E_SUCCESS,
    E_ESCAPE_CHAR = -1,
    E_NOT_OCTAL = -2,
    E_MALLOC = -3
};

enum token {
    CHAR_CONSTANT
};

struct character {
    char c;
};

int convert_single_escape(char c);
int convert_octal_escape(char *seq, int len);
static inline int isodigit(const char c);
struct character *create_character(char c);
void handle_error(enum lexer_error e, char *source);
int emalloc(void **ptr, size_t n);
