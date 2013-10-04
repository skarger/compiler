%{
#include <stdio.h>

#include "src/include/lexer.h"

/* creating the tokens here so the lexer should ignore token.h */
#define TOKEN_H


YYSTYPE yylval;
int yylex(void);
void yyerror(char *s);

%}

%start  conditional_expr

%token UNRECOGNIZED CHAR_CONSTANT STRING_CONSTANT NUMBER_CONSTANT IDENTIFIER
%token BREAK CHAR CONTINUE DO ELSE FOR GOTO IF INT
%token LONG RETURN SIGNED SHORT UNSIGNED VOID WHILE
%token LOGICAL_NOT REMAINDER BITWISE_XOR AMPERSAND ASTERISK MINUS PLUS ASSIGN
%token BITWISE_NOT BITWISE_OR LESS_THAN GREATER_THAN DIVIDE TERNARY_CONDITIONAL
%token LEFT_PAREN RIGHT_PAREN LEFT_BRACKET RIGHT_BRACKET LEFT_BRACE RIGHT_BRACE
%token COMMA SEMICOLON COLON
%token ADD_ASSIGN SUBTRACT_ASSIGN MULTIPLY_ASSIGN DIVIDE_ASSIGN REMAINDER_ASSIGN
%token BITWISE_LSHIFT_ASSIGN BITWISE_RSHIFT_ASSIGN BITWISE_AND_ASSIGN
%token BITWISE_XOR_ASSSIGN BITWISE_OR_ASSIGN
%token INCREMENT DECREMENT BITWISE_LSHIFT BITWISE_RSHIFT
%token LESS_THAN_EQUAL GREATER_THAN_EQUAL EQUAL NOT_EQUAL LOGICAL_AND LOGICAL_OR

%left  '|'
%left  '&'
%left  '+'  '-'
%left  '*'  '/'  '%'
%left  UMINUS      /*  supplies  precedence  for  unary  minus  */

%%      /*  beginning  of  rules  section  */

conditional_expr : cast_expr
        { printf("parsed conditional_expr!\n"); }
    ;

cast_expr : /* empty */
    | unary_expr
    | LEFT_PAREN type_name RIGHT_PAREN cast_expr
    ;

/* unary_expr and children */
unary_expr : postfix_expr
    | MINUS cast_expr
    | PLUS cast_expr
    | LOGICAL_NOT cast_expr
    | BITWISE_NOT cast_expr
    | AMPERSAND cast_expr
    | ASTERISK cast_expr
    | INCREMENT unary_expr
    | DECREMENT unary_expr
    ;

/* postfix_expr and children */
postfix_expr : primary_expr
    | subscript_expr
    | function_call
    | postfix_expr INCREMENT
    | postfix_expr DECREMENT 
    ;

primary_expr : IDENTIFIER 
    | constant
    | LEFT_PAREN expr RIGHT_PAREN
    ;

constant : CHAR_CONSTANT
    | STRING_CONSTANT
    | NUMBER_CONSTANT
    ;

subscript_expr: postfix_expr LEFT_BRACKET expr RIGHT_BRACKET
    ;

function_call : postfix_expr LEFT_PAREN expr RIGHT_PAREN
    ;

expr : expr COMMA assignment_expr
    | assignment_expr
    ;

assignment_expr : conditional_expr
    | unary_expr assignment_op assignment_expr
    ;

assignment_op : ASSIGN
    | ADD_ASSIGN
    | SUBTRACT_ASSIGN
    | MULTIPLY_ASSIGN
    | DIVIDE_ASSIGN
    | REMAINDER_ASSIGN
    | BITWISE_LSHIFT_ASSIGN
    | BITWISE_RSHIFT_ASSIGN
    | BITWISE_AND_ASSIGN
    | BITWISE_XOR_ASSSIGN
    | BITWISE_OR_ASSIGN
    ;

/* type name and children */
type_name : type_specifier
    | abstract_declarator
    ;

type_specifier : integer_type_specifier
    | void_type_specifier
    ;

integer_type_specifier : signed_type_specifier
    | unsigned_type_specifier
    | character_type_specifier
    ;

signed_type_specifier : signed_short_int
        { printf("signed short int\n"); }
    | signed_int
        { printf("signed int\n"); }
    | signed_long_int
        { printf("signed long int\n"); }
    ;

signed_short_int : SHORT
    | SHORT INT
    | SIGNED SHORT
    | SIGNED SHORT INT
    ;

signed_int : INT
    | SIGNED INT
    | SIGNED
    ;

signed_long_int: LONG
    | LONG INT
    | SIGNED LONG
    | SIGNED LONG INT
    ;

unsigned_type_specifier : UNSIGNED SHORT INT
    | UNSIGNED INT
    | UNSIGNED LONG INT
    ;

character_type_specifier : CHAR
    | SIGNED CHAR
    | UNSIGNED CHAR
    ;

void_type_specifier : VOID;


abstract_declarator : pointer
    | pointer direct_abstract_declarator
    ;

pointer : ASTERISK
    | ASTERISK pointer
    ;

direct_abstract_declarator : LEFT_PAREN abstract_declarator RIGHT_PAREN
    | direct_abstract_declarator LEFT_BRACKET conditional_expr RIGHT_BRACKET 
    ;


%%      /*  start  of  programs  */
#include "lex.yy.c"

main() {
  return yyparse();
}

void yyerror(char *s) {
  fprintf(stderr, "%s\n", s);
}

