%{
#include <stdio.h>
#include "lexer.h"
#define YYSTYPE void *
/* extern YYSTYPE yylval; */
YYSTYPE yylval;

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
        yylval = (YYSTYPE) create_character(strtochar(yytext, yyleng));
        return CHAR_CONSTANT;
    }
%%

struct character *create_character(char c) {
    struct character *sc = malloc(sizeof(struct character));
    sc->c = c;
    return sc;
}

/* error conditions should not happen based on regular expressions but check anyway */ 
int strtochar(char *str, int len) {
    int rv;
    int char_seq_len = len - 2;
    char *buf = malloc(char_seq_len + 1);

    if (str[0] != '\'' || str[len - 1] != '\'') {
        /* error: input not wrapped in '' */
        rv = E_INPUT;
        goto cleanup;
    }

    if (len < 3) {
        /* error: empty or malformed character constant */
        rv = E_INPUT;
        goto cleanup;
    }

    /* trim the surrounding '' characters, leaving room for terminating '\0' */
    strncpy(buf, str+1, char_seq_len);
    buf[char_seq_len] = '\0';

    if (char_seq_len == 1) {
        /* normal case: 'a' */
        rv = buf[0];
        goto cleanup;
    }

    if (char_seq_len > 4) {
        /* error: invalid escape sequence */
        /* longest format is 4 chars e.g. '\377' */
        rv = E_INPUT;
        goto cleanup;
    }

    if (buf[0] == '\\') {
        /* escape code */
        if ( isodigit(buf[1]) ) {
            /* length of octal digit sequence is char_seq_len-1 for the leading \ */
            rv = convert_octal_escape(buf+1, char_seq_len-1);
            goto cleanup;
        }
        /* not an octal escape. that implies it must be one escaped character */
        if (char_seq_len != 2) {
            rv = E_INPUT;
            goto cleanup;
        }
        rv = convert_single_escape(buf[1]);
        goto cleanup;
    }
    /* error: reaching here means sequence longer than 1 char and not a valid escape */
    rv = E_INPUT;

    cleanup:
    free(buf);
    return rv;
}

int convert_single_escape(char c) {
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
            /* return c; */
            /* error: our specification does not allow "identity" escapes */
            return E_ESCAPE_CHAR;
    }
}

int convert_octal_escape(char *seq, int len) {
    /* put up to len octal digits from seq into buf */
    /* then convert buf to an octal number and return it as a char */
    int i = 0;
    while (i < len) {
        if (!isodigit(seq[i])) {
            return E_NOT_OCTAL;
        }
        i++;
    }
    return strtol(seq, NULL, 8);
}

/* this convenience function copied from
 * http://comments.gmane.org/gmane.linux.network/265196 */
static inline int isodigit(const char c)
{
    return c >= '0' && c <= '7';
}