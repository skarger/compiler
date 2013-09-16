%option yylineno
%{
#include "lexer.h"
%}

 /* Whitespace is defined as any sequence of the blank (space), new line, */
 /* vertical tab, form feed, and horizontal tab characters. */
whitespace [ \v\f\t]+
newline \r?\n
%%
{whitespace} ;
{newline} { return EOL; }
%%
