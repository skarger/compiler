

int strtochar(char *str, int len);
int convert_single_escape(char c);
int convert_octal_escape(char *seq, int len);
static inline int isodigit(const char c);
struct character *create_character(char c);


enum lexer_error {
    E_INPUT = -1,
    E_ESCAPE_CHAR = -2,
    E_NOT_OCTAL = -3
};

enum token {
    CHAR_CONSTANT
};

struct character {
    char c;
};
