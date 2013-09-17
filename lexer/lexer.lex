/*
 * lexer.lex
 * Specification for the (f)lex generated lexer for the CSCI-E95 source language,
 * which is a simplified version of the ISO C language.
 *
 * Certain tokens are uniquely specified: reserved words, operators, and separators.
 *
 * Other tokens can take on arbitrary values:
 *      identifiers are sequences of letters, digits, and underscores.
 *      constants (also known as literals) are integers, characters, and strings.
 * This file contains helper routines to create structures to be returned to the
 * calling program containing data for those variable-value tokens.
 */

%{
#include <stdio.h>
#include <string.h>
#include <error.h>

#include "lexer.h"


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
'{graphic}'   {
    yylval = (YYSTYPE) create_character(yytext[1]);
    return CHAR_CONSTANT;
}
'{char_esc}'  {
    yylval = (YYSTYPE) create_character( (char) convert_single_escape(yytext[2]) );
    return CHAR_CONSTANT;
}
'{octal_esc}' {
    /* format is '\nnn' with 1-3 n values. the number of n chars is yyleng - 3 */
    char buf[4];
    int num_chars = yyleng - 3;
    strncpy(buf, yytext + 2, num_chars);
    buf[num_chars] = '\0';
    yylval = (YYSTYPE) create_character( (char) convert_octal_escape( buf, num_chars ));
    return CHAR_CONSTANT;
}
%%

/* character constants */


/*
 * create_character
 * Purpose:
 *      Store a character constant in and return it.
 * Parameters:
 *      c - the character value
 * Returns:
 *      A pointer to the struct character containing this constant
 * Side effects:
 *      Allocates memory on the heap
 */
struct character *create_character(char c) {
    struct character *sc;
    emalloc((void **) &sc, sizeof(struct character));
    sc->c = c;
    return sc;
}

/*
 * convert_single_escape
 * Purpose:
 *      Transform a character 'x' into the escaped char value '\x'.
 * Parameters:
 *      c - the character value
 * Returns:
 *      An escape character value. For example, given 'n' return the newline '\n'.
 * Side effects:
 *      None
 */
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
            handle_error(E_ESCAPE_CHAR, "convert_single_escape");
    }
}

/*
 * convert_octal_escape
 * Purpose:
 *      Transform a sequence of octal number chars into an escaped char value.
 * Parameters:
 *      seq - the string of chars, each of which is an octal number
 *      len - the length of the string, i.e. the number of digits
 * Returns:
 *      An escape character value. For example, given "142" return 'b'.
 * Side effects:
 *      None
 */
int convert_octal_escape(char *seq, int len) {
    /* put up to len octal digits from seq into buf */
    /* then convert buf to an octal number and return it as a char */
    int i = 0;
    while (i < len) {
        if (!isodigit(seq[i])) {
            handle_error(E_NOT_OCTAL, "convert_octal_escape");
        }
        i++;
    }
    return strtol(seq, NULL, 8);
}

/* helpers */

/*
 * emalloc
 * Purpose:
 *      Allocate heap memory and store it in the passed in pointer.
 * Parameters:
 *      ptr - a pointer to the pointer that should be set with malloc's return value.
 *      n - the number of bytes to allocate.
 * Returns:
 *      An escape character value. For example, given "142" return 'b'.
 * Side effects:
 *      Terminates program if malloc errors.
 */
int emalloc(void **ptr, size_t n) {
    if ( (*ptr = malloc(n)) == NULL ) {
        handle_error(E_MALLOC, "emalloc");
    }
    return E_SUCCESS;
}

/*
 * handle_error
 * Purpose:
 *      Handle an error caught in the calling method.
 * Parameters:
 *      e - the error value that will become the program's exit status
 *      source - the name of the calling method that will be printed to stderr
 * Returns:
 *      None
 * Side effects:
 *      Terminates program unless e == E_SUCCESS
 */
void handle_error(enum lexer_error e, char *source) {
    switch (e) {
        case E_SUCCESS:
            return;
        case E_MALLOC:
            error(e, 0, "%s: out of memory", source);
        case E_NOT_OCTAL:
            error(e, 0, "%s: non-octal digit", source);
        case E_ESCAPE_CHAR:
            error(e, 0, "%s: invalid escape char", source);
        default:
            return;
    }
}

/* this convenience function copied from
 * http://comments.gmane.org/gmane.linux.network/265196 */
static inline int isodigit(const char c)
{
    return c >= '0' && c <= '7';
}