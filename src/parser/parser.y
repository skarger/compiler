%{
#include <stdio.h>

#include "src/include/lexer.h"
#include "src/include/parser.h"
#include "src/include/utilities.h"

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
        { $$ = create_data_node( IDENTIFIER, yylval ); traverse_data_node($$); }
    | constant
        { $$ = $1; traverse_data_node($$); }
    | LEFT_PAREN expr RIGHT_PAREN
    ;

constant : CHAR_CONSTANT
        { $$ = create_data_node( CHAR_CONSTANT, yylval ); }
    | STRING_CONSTANT
        { $$ = create_data_node( STRING_CONSTANT, yylval ); }
    | NUMBER_CONSTANT
        { $$ = create_data_node( NUMBER_CONSTANT, yylval ); }
    ;

subscript_expr: postfix_expr LEFT_BRACKET expr RIGHT_BRACKET
    ;

function_call : postfix_expr LEFT_PAREN RIGHT_PAREN
    | postfix_expr LEFT_PAREN expr RIGHT_PAREN
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
    | type_specifier abstract_declarator
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

void_type_specifier : VOID
    {  $$ = create_one_item_node(TYPE_SPECIFIER, (int) VOID); traverse_node($$);}


abstract_declarator : pointer { traverse_node($1); }
    | pointer direct_abstract_declarator
    | direct_abstract_declarator
    ;

pointer : ASTERISK
        {  $$ = create_zero_item_node((int) POINTER); }
    | ASTERISK pointer
        {
            Node *n = (Node *) create_zero_item_node((int) POINTER);
            n->children[RIGHT] = $2;
            $$ = n;
        }
    ;

direct_abstract_declarator : LEFT_PAREN abstract_declarator RIGHT_PAREN
    | direct_abstract_declarator LEFT_BRACKET conditional_expr RIGHT_BRACKET
    | direct_abstract_declarator LEFT_BRACKET RIGHT_BRACKET
    | LEFT_BRACKET conditional_expr RIGHT_BRACKET
    | LEFT_BRACKET RIGHT_BRACKET
    ;


%%      /*  start  of  programs  */
#include "lex.yy.c"

main() {
  return yyparse();
}

void yyerror(char *s) {
  fprintf(stderr, "%s\n", s);
}


void *create_data_node(enum node_type n_type, void *data) {
    Node *n;
    util_emalloc((void **) &n, sizeof(Node));
    n->n_type = n_type;
    /* populate the data field, accounting for the type of data */
    switch (n_type) {
        case IDENTIFIER:
            n->data.values.str = strdup( ((struct String *) data)->str );
            break;
        case STRING_CONSTANT:
            n->data.values.str = strdup( ((struct String *) data)->str );
            break;
        case NUMBER_CONSTANT:
            n->data.values.num = ((struct Number *) data)->value;
            break;
        case CHAR_CONSTANT:
            n->data.values.ch = ((struct Character *) data)->c;
            break;
        default:
            handle_parser_error(PE_INVALID_DATA_TYPE,
                                get_token_name((int) n_type), yylineno);
            free(n);
            return NULL;
    }
    return (void *) n;
}

void traverse_data_node(void *np) {
    Node *n = (Node *) np;
    switch (n->n_type) {
        case IDENTIFIER:
            printf("%s", n->data.values.str);
            break;
        case STRING_CONSTANT:
            /* TODO: replace special characters, e.g. replace newline with \n */
            printf("\"%s\"", n->data.values.str);
            break;
        case NUMBER_CONSTANT:
            printf("%d", n->data.values.num);
            break;
        case CHAR_CONSTANT:
            /* TODO: replace special characters, e.g. replace null with \0 */
            printf("'%c'", n->data.values.ch);
            break;
        default:
            handle_parser_error(PE_INVALID_DATA_TYPE,
                                get_token_name((int) n->n_type), yylineno);
            break;
        }
}

void *create_node(enum node_type nt) {
    Node *n;
    util_emalloc((void **) &n, sizeof(Node));
    n->n_type = nt;
    return n;
}

void initialize_children(Node *n) {
    int i;
    for (i = 0; i < MAX_CHILDREN; i++) {
        n->children[i] = NULL;
    }
}

void *create_zero_item_node(enum node_type nt) {
    Node *n = create_node(nt);
    initialize_children(n);
    return (void *) n;
}

void *create_one_item_node(enum node_type nt, int item1) {
    Node *n = create_node(nt);
    n->data.symbols[0] = item1;
    initialize_children(n);
    return (void *) n;
}

void traverse_node(void *np) {
    Node *n = (Node *) np;
    if (n == NULL) {
        return;
    }
    switch (n->n_type) {
        case POINTER:
            printf("*");
            traverse_node((void *) n->children[RIGHT]);
            break;
        case TYPE_SPECIFIER:
            printf("%s", get_type_name(n->data.symbols[TYPE]));
            break;
        default:
            error(1, 0, "unknown node type");
            break;
    }
}


void pretty_print(Node *n) {
    switch (n->n_type) {
        case TYPE_SPECIFIER:
            printf("%s ", get_type_name(n->data.symbols[TYPE]));
            if (n->children[RIGHT] != NULL) {
                pretty_print(n->children[RIGHT]);
            }
            break;
        default:
            error(1, 0, "unknown node type");
            break;
    }
}

char *get_type_name(enum data_type type) {
    switch (type) {
        case VOID:
            return "void";
        default:
            return "";
    }
}

/*
 * handle_parser_error
 * Purpose:
 *      Handle an error caught in the calling method.
 * Parameters:
 *      e - the error value.
 *      data - string that will be inserted into the message printed to stderr
 *      line - line number causing error, if applicable (e.g. from input source)
 * Returns:
 *      None
 * Side effects:
 *      May terminate program depending on error type
 */
void handle_parser_error(enum parser_error e, char *data, int line) {
    switch (e) {
        case PE_SUCCESS:
            return;
        case PE_INVALID_DATA_TYPE:
            error(0, 0, "line %d: invalid data type: %s", line, data);
            return;
        default:
            return;
    }
}
