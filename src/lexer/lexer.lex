/*
 * lexer.lex
 * Specification for the (f)lex generated lexer for the CSCI-E95 source
 * language, which is a simplified version of the ISO C language.
 *
 * Certain tokens are uniquely specified: reserved words, operators, separators.
 *
 * Other tokens can take on arbitrary values:
 *  identifiers are sequences of letters, digits, and underscores.
 *  constants (also known as literals) are integers, characters, and strings.
 * This file contains helper routines to create structures to be returned to the
 * calling program containing data for those variable-value tokens.
 */

%option noyywrap
%option nounput
%option yylineno

%x STRING

%{
#include <stdio.h>
#include <string.h>
#ifdef __linux
#include <error.h>
#endif

#include "src/include/lexer.h"
#include "src/include/literal.h"
#include "src/include/parse-tree.h"

#ifdef STANDALONE
#include "src/include/token.h"
#else
#include "y.tab.h"
#endif



extern YYSTYPE yylval;
/* YYSTYPE yylval; */

/* helpers */
int convert_single_escape(char c);
int convert_octal_escape(char *seq, int n_digits);
static inline int isodigit(const char c);
void handle_error(enum lexer_error e, char *data, int line);
void emalloc(void **ptr, size_t n);

%}
 /* basic chars */
letter [A-Za-z]
digit [0-9]
sp [ ]
 /* rules below rely on ws not including newlines */
ws [ \v\f\t]
nl \r?\n

 /* graphic chars: */
 /* ! # % ^ & * ( ) - _ + = ~ [ ] \ | ; : ' " { } , . < > / ? $ @ ` */
 /* keep quote and apostrophe separate because of their use in constants */
graphic [!#%^&*()\\\-_+=~\[\]|;:'"{},.<>/?$@`]
graphic_no_quote [!#%^&*()\\\-_+=~\[\]|;:'{},.<>/?$@`]
graphic_no_apostrophe [!#%^&*()\\\-_+=~\[\]|;:"{},.<>/?$@`]


 /* octal escapes
  * From the C specification:
  * "The value of the octal or hexadecimal escape sequence must be in the range 
  * of representable values for type unsigned char for a character constant and
  * type wchar_t for a wide-character constant."
  * This lexer does not accomodate wide-characters.
  */
octal_esc \\[0-3]?[0-7]?[0-7]
 /* character escape codes may be n, t, b, r, f, v, \, ', ", a, and ?. */
char_esc \\[ntbrfv\\'"a?]
invalid_esc \\[^ntbrfv\\'"a?0-7]

 /* comments */
start_comment \/\*
end_comment \*\/
chars_within_comment ([^*])|(\*[^/])
comment {start_comment}({chars_within_comment})*{end_comment}

%%

{nl}+ ; /* do nothing with newline, using yylineno */
{ws}+ ; /* do nothing */
{comment} /* do nothing */

 /* reserved words begin */
break return BREAK;
char return CHAR;
continue return CONTINUE;
do return DO;
else return ELSE;
for return FOR;
goto return GOTO;
if return IF;
int return INT;
long return LONG;
return return RETURN;
signed return SIGNED;
short return SHORT;
unsigned return UNSIGNED;
void return VOID;
while return WHILE;
 /* reserved words end */

 /* identifiers begin */
(_|{letter})(_|{letter}|{digit})* {
    yylval = (YYSTYPE) create_string(yyleng);
    strncpy( ((struct String *) yylval)->str, yytext, yyleng );
    ((struct String *) yylval)->current =
        ((struct String *) yylval)->str + yyleng;
    *( ((struct String *) yylval)->current ) = '\0';
    return IDENTIFIER;
}
 /* error on string that starts with a number but looks like an identifier */
({digit}+)(_|{letter})+(_|{letter}|{digit})* {
    handle_error(E_INVALID_ID, yytext, yylineno);
    return UNRECOGNIZED;
}
 /* identifiers end */

 /* operators and separators begin */
 /* must escape: " \ [ ] ^ - ? . * + | ( ) $ / { } % < > */

 /* simple operators */
 /*   !   %   ^   &   *   -   +   =   ~   |   <   >   /   ?   */
!  return LOGICAL_NOT;
\% return REMAINDER;
\^ return BITWISE_XOR;
&  return AMPERSAND;
\* return ASTERISK;
\- return MINUS;
\+ return PLUS;
=  return ASSIGN;
~  return BITWISE_NOT;
\| return BITWISE_OR;
\< return LESS_THAN;
\> return GREATER_THAN;
\/ return DIVIDE;
\? return TERNARY_CONDITIONAL;

 /* compound assignment operators */
 /*  +=  -=  *=  /=  %=  <<=  >>=  &=  ^=  |=  */
\+=   return ADD_ASSIGN;
\-=   return SUBTRACT_ASSIGN;
\*=   return MULTIPLY_ASSIGN;
\/=   return DIVIDE_ASSIGN;
\%=   return REMAINDER_ASSIGN;
\<\<= return BITWISE_LSHIFT_ASSIGN;
\>\>= return BITWISE_RSHIFT_ASSIGN;
&=    return BITWISE_AND_ASSIGN;
\^=   return BITWISE_XOR_ASSSIGN;
\|=   return BITWISE_OR_ASSIGN;

 /* other compound operators */
 /*  ++  --  <<  >>  <=  >=  ==  !=  &&  ||  */
\+\+ return INCREMENT;
\-\- return DECREMENT;
\<\< return BITWISE_LSHIFT;
\>\> return BITWISE_RSHIFT;
\<=  return LESS_THAN_EQUAL;
\>=  return GREATER_THAN_EQUAL;
==   return EQUAL;
!=   return NOT_EQUAL;
&&   return LOGICAL_AND;
\|\| return LOGICAL_OR;

 /* other separators begin */
 /*   (   )   [   ]   {   }   ,   ;   :   */
\( return LEFT_PAREN;
\) return RIGHT_PAREN;
\[ return LEFT_BRACKET;
\] return RIGHT_BRACKET;
\{ return LEFT_BRACE;
\} return RIGHT_BRACE;
,  return COMMA;
;  return SEMICOLON;
:  return COLON;
 /* other separators end */

 /* operators and separators end */

 /* integral constants begin */
0 |
[1-9][0-9]* {
    yylval = (YYSTYPE) create_number(yytext);
    if ( ((struct Number *) yylval)->type == INTEGER_OVERFLOW ) {
        handle_error(E_INTEGER_OVERFLOW, yytext, yylineno);
        return UNRECOGNIZED;
    }
    return NUMBER_LITERAL;
}
0[0-9]+ {
    handle_error(E_OCTAL, yytext, yylineno);
    return UNRECOGNIZED;
}
[0-9]+\.[0-9]* |
[0-9]*\.[0-9]+ {
    handle_error(E_FLOAT, yytext, yylineno);
    return UNRECOGNIZED;
}
 /* integral constants end */

 /* character constants begin */
'{letter}'                |
'{digit}'                 |
'{sp}'                    |
'{graphic_no_apostrophe}' {
    yylval = (YYSTYPE) create_character(yytext[1]);
    return CHAR_LITERAL;
}
'{char_esc}'  {
    yylval =
        (YYSTYPE) create_character( (char) convert_single_escape(yytext[2]) );
    return CHAR_LITERAL;
}
'{octal_esc}' {
    /* minus the two apostrophes and slash, num of octal digits is len - 3 */
    int n_digits = yyleng - 3;
    /* the first octal digit is after the first ' and the \ */
    char *start = yytext + 2;
    yylval =
        (YYSTYPE) create_character((char)convert_octal_escape(start, n_digits));
    return CHAR_LITERAL;
}
'{invalid_esc}' {
    handle_error(E_ESCAPE_SEQ, yytext, yylineno);
    return UNRECOGNIZED;
}
 /* any char not matched above is not part of the accepted character set */
''|'''|'[^']+' {
    if (yyleng == 2) {
        /* we have '' */
        handle_error(E_EMPTY_CHAR, yytext, yylineno);
        return UNRECOGNIZED;
    } else {
        handle_error(E_INVALID_CHAR, yytext, yylineno);
        return UNRECOGNIZED;
    }
}

 /* character constants end */

 /* string constants begin */
\"([^"]|\\\")*\" {
    BEGIN(STRING);
    /* create storage for string literal and then push it back for re-scanning
     * do not push back the leading quote though so the re-scan starts
     * with the first content char (or the trailing " for an empty string)
     */
    yylval = (YYSTYPE) create_string(yyleng);
    yyless(1);
}

<STRING>{letter}|{digit}|{ws}|{graphic_no_quote} {
    /* this is tricky:
     * yylval points to a struct String that we created
     *      upon finding the string literal
     * yylval's current member points to the end of its str member
    *       where we want to append
     * so we append yytext's value and then increment current
     */
    *( ((struct String *) yylval)->current++ ) = *yytext;
}
<STRING>{char_esc} {
    /* yytext is something like \n */
    *( ((struct String *) yylval)->current++ ) =
        (char) convert_single_escape(yytext[1]);
}
<STRING>{octal_esc} {
    /* yytext is something like \377 */
    /* subtracting the slash, the num of octal digits is len - 1 */
    int n_digits = yyleng - 1;
    char *start = yytext + 1; /* the first octal digit is after the \ */
    *( ((struct String *) yylval)->current++ ) = 
        (char) convert_octal_escape( start, n_digits );
}
<STRING>{nl} {
    /* copy the invalid newline to the string but mark it as invalid */
    ((struct String *) yylval)->valid = FALSE;
    if (*yytext == '\r') {
        *( ((struct String *) yylval)->current++ ) = *yytext++;
    }
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
<STRING>[^"] {
    /* any other character is not in the accepted input character set */
    /* copy the char to the string but mark it as invalid */
    ((struct String *) yylval)->valid = FALSE;
    *( ((struct String *) yylval)->current++ ) = *yytext;
    handle_error(E_INVALID_CHAR, yytext, yylineno);
}
<STRING>\" {
    /* if we're in a string then a non-escaped " means end of string */
    *( ((struct String *) yylval)->current ) = '\0';
    BEGIN(0);
    if (((struct String *) yylval)->valid) {
        return STRING_LITERAL;
    }
    /* error found somewhere in string */
    handle_error(E_INVALID_STRING, ((struct String *) yylval)->str, yylineno);
    return UNRECOGNIZED;
}
 /* string constants end */

. return UNRECOGNIZED;
%%

/* integral constants */

/*
 * create_number
 * Purpose:
 *      Construct a number.
 * Parameters:
 *      digit_str - string of digits representing the number.
 * Returns:
 *      A pointer to the struct Number containing the value and integer type.
 *      The type member will be set to the minimum size required by the value.
 *      The type is assumed to be signed unless the value implies unsigned.
 * Side effects:
 *      Allocates memory on the heap.
*/
struct Number *create_number(char *digit_str) {
    struct Number *n;
    emalloc((void **) &n, sizeof(struct Number));
    errno = 0;
    n->value = strtoul(digit_str, NULL, 10);
    /* default n->type to signed unless the value implies unsigned */
    /* the parser can change n->type to unsigned if specified */
    if (ERANGE == errno || n->value > 4294967295ul) {
        /*
        * Integer constant was too large for unsigned long. value will be
        * MAX_ULONG, as defined by strtoul.
        */
        n->type = INTEGER_OVERFLOW;
    }  else if (n->value > 2147483647) {
        n->type = UNSIGNED_LONG;
    } else {
        /* 65535 is the largest SIGNED_SHORT */
        /* but just call it an int */
        n->type = SIGNED_INT;
    }

    return n;
}

 /* string constants */

/*
 * create_string
 * Purpose:
 *      Construct a string.
 * Parameters:
 *      len - the length of string. This function will allocate memory for
 *          len characters plus a null byte.
 * Returns:
 *      A pointer to the struct String. The str member will be initially null.
 * Side effects:
 *      Allocates memory on the heap.
 */
struct String *create_string(int len) {
    struct String *s;
    emalloc((void **) &s, sizeof(struct String));
    emalloc((void **) &(s->str), len + 1);
    /* initialize to the empty string */
    *(s->str) = '\0';
    /* current is used to append chars so set it equal to str initially */
    s->current = s->str;
    s->valid = TRUE;
    return s;
}


/* character constants */

/*
 * create_character
 * Purpose:
 *      Store a character constant in and return it.
 * Parameters:
 *      c - the character value.
 * Returns:
 *      A pointer to the struct Character containing this constant.
 * Side effects:
 *      Allocates memory on the heap.
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
 *      An escape character value. E.g. given 'n' return the newline '\n'.
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
 *      Transform an octal escape sequence string into the intended char value.
 * Parameters:
 *      seq - the sequence of chars in the form 377, with 1-3 octal digit values
 *      len - the number of digits
 * Returns:
 *      An escape character value. For example, given "142" return 'b'.
 * Side effects:
 *      None
 */
int convert_octal_escape(char *seq, int n_digits) {
    int i = 0;
    /* create a string with only the digits */
    char buf[4]; /* room for up to three digits and null byte */
    strncpy(buf, seq, n_digits);
    buf[n_digits] = '\0';

    /* convert digit string to an octal number and return it as a char */
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
 *      ptr - pointer to pointer that should be set to malloc's return value.
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
 *      e - the error value. if non-zero the program will exit with status e
 *      data - string that will be inserted into the message printed to stderr
 *      line - line number causing error, if applicable (e.g. from input source)
 * Returns:
 *      None
 * Side effects:
 *      Terminates program unless e == E_SUCCESS
 */
void handle_error(enum lexer_error e, char *data, int line) {
    switch (e) {
        case E_SUCCESS:
            return;
#ifdef __linux
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
            error(0, 0, "line %d: invalid newline", line);
            return;
        case E_INVALID_STRING:
            error(0, 0, "line %d: invalid string literal: %s", line, data);
            return;
        case E_INVALID_ID:
            error(0, 0, "line %d: invalid identifier: %s", line, data);
            return;
        case E_INVALID_CHAR:
            error(0, 0, "line %d: invalid character: %s", line, data);
            return;
        case E_EMPTY_CHAR:
            error(0, 0, "line %d: empty character constant: %s", line, data);
            return;
        case E_OCTAL:
            error(0, 0, "line %d: octal constants unsupported: %s", line, data);
            return;
        case E_FLOAT:
            error(0, 0, "line %d: floating point unsupported: %s", line, data);
            return;
        case E_INTEGER_OVERFLOW:
            error(0, 0, "line %d: integer constant too large: %s", line, data);
            return;
#else
        case E_MALLOC:
            fprintf(stderr, "%s: out of memory\n", data);
            return;
        case E_NOT_OCTAL:
            fprintf(stderr, "line %d: %s: non-octal digit\n", line, data);
            return;
        case E_ESCAPE_SEQ:
            fprintf(stderr, "line %d: invalid escape sequence %s\n", line, data);
            return;
        case E_NEWLINE:
            fprintf(stderr, "line %d: invalid newline\n", line);
            return;
        case E_INVALID_STRING:
            fprintf(stderr, "line %d: invalid string literal: %s\n", line, data);
            return;
        case E_INVALID_ID:
            fprintf(stderr, "line %d: invalid identifier: %s\n", line, data);
            return;
        case E_INVALID_CHAR:
            fprintf(stderr, "line %d: invalid character: %s\n", line, data);
            return;
        case E_EMPTY_CHAR:
            fprintf(stderr, "line %d: empty character constant: %s\n", line, data);
            return;
        case E_OCTAL:
            fprintf(stderr, "line %d: octal constants unsupported: %s\n", line, data);
            return;
        case E_FLOAT:
            fprintf(stderr, "line %d: floating point unsupported: %s\n", line, data);
            return;
        case E_INTEGER_OVERFLOW:
            fprintf(stderr, "line %d: integer constant too large: %s\n", line, data);
            return;
#endif
        default:
            return;
    }
}

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
        CASE_FOR(CHAR_LITERAL);
        CASE_FOR(STRING_LITERAL);
        CASE_FOR(NUMBER_LITERAL);
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

/* this convenience function copied from
 * http://comments.gmane.org/gmane.linux.network/265196 */
static inline int isodigit(const char c) {
    return c >= '0' && c <= '7';
}
