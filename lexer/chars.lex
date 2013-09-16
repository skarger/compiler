%{
#include <stdio.h>

char strtochar(char *str, int len);

%}
letter [A-Za-z]
digit [0-9]
sp [ ]
ws [ \v\f\t]
nl \r?\n

 /* graphic chars: ! # % ^ & * ( ) \ - _ + = ~ [ ] \ | ; : ' " { } , . < > / ? $ @ ` */
graphic [!#%^&*()\-_+=~\[\]|;:\"\{\},.<>\/?$@`]
apostrophe '
backslash \\

 /* From the C specification:
  * "The value of the octal or hexadecimal escape sequence must be in the range of 
  * representable values for type unsigned char for a character constant and type wchar_t 
  * for a wide-character constant."
  * This lexer does not accomodate wide-characters
  */
octal_esc \\[0-3]?[0-7]?[0-7]
 /* Character escape code may be n, t, b, r, f, v, \, ', ", a, and ?.
  * Notably this does not allow the "identity" escape of non-codes, for example '\c'
  */
char_esc \\[ntbrfv\\'"a?]
%%
'{letter}'    |
'{digit}'     |
'{sp}'        |
'{graphic}'   |
'{backslash}{apostrophe}' |
'{backslash}{backslash}'  |
'{char_esc}'  |
'{octal_esc}' {
        printf("got a char: %c|\n", strtochar(yytext, yyleng));
    }
%%

char strtochar(char *str, int len);
char convert_single_escape(char c);
char convert_octal_escape(char *seq);
static inline int isodigit(const char c);

/* error conditions should not happen based on regular expressions but check anyway */ 
char strtochar(char *str, int len) {
    if (len < 3) {
        /* error: empty or malformed character constant */
        return '\0';
    }
    if (len == 3) {
        /* normal case: 'a' */
        return str[1];
    }
    /* len > 3 */
    if (str[1] == '\\') {
        /* escape code */
        if (len > 6) {
            /* error: invalid escape sequence */
            /* longest at 6 chars is in the form '\377' */
            return '\0';
        }

        if (isodigit(str[2])) {
            return convert_octal_escape(str + 2);
        }

        /* not an octal escape. that implies it must be be one character => len == 4 */
        if (len == 4) {
            return convert_single_escape(str[2]);
        }
    }

    /* error: sequence longer than 1 char and not a valid escape */
    return '\0';
}

char convert_single_escape(char c) {
    switch (c) {
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case 'b':
            return '\b';
        case 'r':
            return '\r';
        case 'f':
            return '\f';
        case 'v':
            return '\v';
        case '\\':
            return '\\';
        case '\'':
            return '\'';
        case '"':
            return '\"';
        case 'a':
            return '\a';
        case '?':
            return '\?';
        default:
            /* maybe error: our specification does not allow "identity" escapes */
            return c;
    }
}

char convert_octal_escape(char *seq) {
    /* put up to 3 octal digits from seq into buf */
    /* then convert buf to an octal number and return it as a char */
    int i = 0;
    char buf[4];
    printf("cos: %s\n", seq);
    while (i < 3 && isodigit(seq[i])) {
        buf[i] = seq[i];
        i++;
    }
    buf[i] = '\0';
    return (char) strtol(buf, NULL, 8);
}

/* this convenience function copied from
 * http://comments.gmane.org/gmane.linux.network/265196 */
static inline int isodigit(const char c)
{
    return c >= '0' && c <= '7';
}