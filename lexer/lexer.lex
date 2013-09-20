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

%option noyywrap
%option yylineno

%x STRING

%{
#include <stdio.h>
#include <string.h>
#include <error.h>

#include "lexer.h"


/* extern YYSTYPE yylval; */
YYSTYPE yylval;

struct String *create_string(int len);

%}

letter [A-Za-z]
digit [0-9]
sp [ ]
ws [ \v\f\t]
nl \r?\n

 /* graphic chars: ! # % ^ & * ( ) \ - _ + = ~ [ ] \ | ; : ' " { } , . < > / ? $ @ ` */
graphic [!#%^&*()\\\-_+=~\[\]|;:"{},.<>/?$@`]

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

invalid_esc \\[^ntbrfv\\'"a?0-7]

%%
 /* rules for character constants */
{nl}+ ; /* do nothing with newline, using yylineno */
{ws}+ ; /* do nothing */

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
    /* subtracting the two apostrophes and slash, the num of octal digits is len - 3 */
    int n_digits = yyleng - 3;
    char *start = yytext + 2; /* the first octal digit is after the first ' and the \ */
    yylval = (YYSTYPE) create_character( (char) convert_octal_escape( start, n_digits ));
    return CHAR_CONSTANT;
}
'{invalid_esc}' {
    handle_error(E_ESCAPE_SEQ, yytext, yylineno);
    return UNRECOGNIZED;
}

 /* rules for string constants */
\"([^"]|\\\")*\" {
    BEGIN(STRING);
    /* create storage for string literal and then push it back for re-scanning
     * do not push back the leading quote though so the re-scan starts
     * with the first content char (or the trailing " for an empty string)
     */
    yylval = (YYSTYPE) create_string(yyleng);
    yyless(1);
}
<STRING>[^"\n\\] {
    /* this is tricky:
     * yylval points to a struct String that we created upon finding the string literal
     * yylval's current member points to the end of its str member where we want to append
     * so we append yytext's value and increment current to where we want to append next
     */
    *( ((struct String *) yylval)->current++ ) = *yytext;
}
<STRING>{char_esc} {
    /* yytext is something like \n */
    *( ((struct String *) yylval)->current++ ) = (char) convert_single_escape(yytext[1]);
}
<STRING>{octal_esc} {
    /* yytext is something like \377 */
    /* subtracting the slash, the num of octal digits is len - 1 */
    int n_digits = yyleng - 1;
    char *start = yytext + 1; /* the first octal digit is after the \ */
    *( ((struct String *) yylval)->current++ ) = 
        (char) convert_octal_escape( start, n_digits );
}
<STRING>\n {
    /* copy the invalid newline to the string but mark it as invalid */
    ((struct String *) yylval)->valid = FALSE;
    *( ((struct String *) yylval)->current++ ) = *yytext;
    handle_error(E_NEWLINE, "", yylineno);
}
<STRING>{invalid_esc} {
    /* copy the invalid escape sequence to the string but mark it as invalid */
    ((struct String *) yylval)->valid = FALSE;
    *( ((struct String *) yylval)->current++ ) = yytext[0];
    *( ((struct String *) yylval)->current++ ) = yytext[1];
    handle_error(E_ESCAPE_SEQ, "", yylineno);
}
<STRING>\" {
    /* if we're in a string then a non-escaped " means end of string */
    *( ((struct String *) yylval)->current ) = '\0';
    BEGIN(0);
    if (((struct String *) yylval)->valid) {
        return STRING_CONSTANT;
    }
    /* error found somewhere in string */
    handle_error(E_INVALID_STRING, ((struct String *) yylval)->str, yylineno);
    return UNRECOGNIZED;
}

. return UNRECOGNIZED;
%%

 /* string constants */

struct String *create_string(int len) {
    struct String *ss;
    emalloc((void **) &ss, sizeof(struct String));
    emalloc((void **) &(ss->str), len + 1);
    /* current is used to append chars to string so set it equal to str initially */
    ss->current = ss->str;
    ss->valid = TRUE;
    return ss;
}


/* character constants */

/*
 * create_character
 * Purpose:
 *      Store a character constant in and return it.
 * Parameters:
 *      c - the character value
 * Returns:
 *      A pointer to the struct Character containing this constant
 * Side effects:
 *      Allocates memory on the heap
 */
struct Character *create_character(char c) {
    struct Character *sc;
    emalloc((void **) &sc, sizeof(struct Character));
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
            /* error: not a supported escape. */
            return E_ESCAPE_SEQ;
    }
}

/*
 * convert_octal_escape
 * Purpose:
 *      Transform a string representing an octal escape sequence into the char value.
 * Parameters:
 *      seq - the sequence of chars in the form 377, with 1 to 3 octal digit values
 *      len - the number of digits
 * Returns:
 *      An escape character value. For example, given "142" return 'b'.
 * Side effects:
 *      None
 */
int convert_octal_escape(char *seq, int n_digits) {
    /* create a string with only the digits */
    char buf[4]; /* room for up to three digits and null byte */
    strncpy(buf, seq, n_digits);
    buf[n_digits] = '\0';

    /* convert digit string to an octal number and return it as a char */
    int i = 0;
    while (i < n_digits) {
        if (!isodigit(buf[i])) {
            handle_error(E_NOT_OCTAL, "convert_octal_escape", 0);
        }
        i++;
    }
    return strtol(buf, NULL, 8);
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
 *      None
 * Side effects:
 *      Sets the value of ptr.
 *      Terminates program if malloc errors.
 */
void emalloc(void **ptr, size_t n) {
    if ( (*ptr = malloc(n)) == NULL ) {
        handle_error(E_MALLOC, "lexer", 0);
    }
}

/*
 * handle_error
 * Purpose:
 *      Handle an error caught in the calling method.
 * Parameters:
 *      e - the error value. if non-zero the program will exit with e as the status
 *      data - string that will be inserted into the message printed to stderr
 *      line - line number that caused error, if applicable (e.g. from input source)
 * Returns:
 *      None
 * Side effects:
 *      Terminates program unless e == E_SUCCESS
 */
void handle_error(enum lexer_error e, char *data, int line) {
    switch (e) {
        case E_SUCCESS:
            return;
        case E_MALLOC:
            error(e, 0, "%s: out of memory", data);
            return;
        case E_NOT_OCTAL:
            error(0, 0, "line %d: %s: non-octal digit", line, data);
            return;
        case E_ESCAPE_SEQ:
            error(0, 0, "line %d: invalid escape sequence %s", line, data);
            return;
        case E_NEWLINE:
            error(0, 0, "line %d: invalid newline", line, data);
            return;
        case E_INVALID_STRING:
            error(0, 0, "line %d: invalid string literal: %s", line, data);
            return;
        default:
            return;
    }
}

/* this convenience function copied from
 * http://comments.gmane.org/gmane.linux.network/265196 */
static inline int isodigit(const char c) {
    return c >= '0' && c <= '7';
}