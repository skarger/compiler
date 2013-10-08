%{
#include <stdio.h>
#include <stdarg.h>

#include "src/include/lexer.h"
#include "src/include/parser.h"
#include "src/include/utilities.h"

/* creating the tokens here so the lexer should ignore token.h */
#define TOKEN_H


YYSTYPE yylval;
int yylex(void);
void yyerror(char *s);

%}

%start  translation_unit

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

translation_unit : conditional_expr SEMICOLON { printf("\n"); }
    | translation_unit conditional_expr
    ;

conditional_expr : cast_expr
    ;

cast_expr : unary_expr
    | LEFT_PAREN type_name RIGHT_PAREN cast_expr
        { traverse_node($2); }
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
        { $$ = create_node( IDENTIFIER, yylval ); }
    | constant
        { $$ = $1; }
    | LEFT_PAREN expr RIGHT_PAREN
    ;

constant : CHAR_CONSTANT
        { $$ = create_node( CHAR_CONSTANT, yylval ); }
    | STRING_CONSTANT
        { $$ = create_node( STRING_CONSTANT, yylval ); }
    | NUMBER_CONSTANT
        { $$ = create_node( NUMBER_CONSTANT, yylval ); }
    ;

subscript_expr: postfix_expr LEFT_BRACKET expr RIGHT_BRACKET
    ;

function_call : postfix_expr LEFT_PAREN RIGHT_PAREN
    | postfix_expr LEFT_PAREN expr RIGHT_PAREN
    ;

expr : comma_expr { traverse_node($$); }
    ;

comma_expr : assignment_expr
    | comma_expr COMMA assignment_expr
    {
         $$ = create_binary_expr_node(COMMA, $1, $3);
    }
    ;

assignment_expr : conditional_expr
    | unary_expr ASSIGN assignment_expr
        { $$ = create_binary_expr_node(ASSIGN, $1, $3); }
    | unary_expr ADD_ASSIGN assignment_expr
        { $$ = create_binary_expr_node(ADD_ASSIGN, $1, $3); }
    | unary_expr SUBTRACT_ASSIGN assignment_expr
        { $$ = create_binary_expr_node(SUBTRACT_ASSIGN, $1, $3); }
    | unary_expr MULTIPLY_ASSIGN assignment_expr
        { $$ = create_binary_expr_node(MULTIPLY_ASSIGN, $1, $3); }
    | unary_expr DIVIDE_ASSIGN assignment_expr
        { $$ = create_binary_expr_node(DIVIDE_ASSIGN, $1, $3); }
    | unary_expr REMAINDER_ASSIGN assignment_expr
        { $$ = create_binary_expr_node(REMAINDER_ASSIGN, $1, $3); }
    | unary_expr BITWISE_LSHIFT_ASSIGN assignment_expr
        { $$ = create_binary_expr_node(BITWISE_LSHIFT_ASSIGN, $1, $3); }
    | unary_expr BITWISE_RSHIFT_ASSIGN assignment_expr
        { $$ = create_binary_expr_node(BITWISE_RSHIFT_ASSIGN, $1, $3); }
    | unary_expr BITWISE_AND_ASSIGN assignment_expr
        { $$ = create_binary_expr_node(BITWISE_AND_ASSIGN, $1, $3); }
    | unary_expr BITWISE_XOR_ASSSIGN assignment_expr
        { $$ = create_binary_expr_node(BITWISE_XOR_ASSSIGN, $1, $3); }
    | unary_expr BITWISE_OR_ASSIGN assignment_expr
        { $$ = create_binary_expr_node(BITWISE_OR_ASSIGN, $1, $3); }
    ;

/* type name and children */
type_name : type_specifier
    | type_specifier abstract_declarator
        {
            Node *n = create_node(TYPE_NAME);
            append_child(n, (Node *) $1, LEFT);
            append_child(n, (Node *) $2, RIGHT);
            $$ = n;
        }
    ;

type_specifier : integer_type_specifier
    | void_type_specifier
    ;

integer_type_specifier : signed_type_specifier
    | unsigned_type_specifier
    | character_type_specifier
    ;

signed_type_specifier : signed_short_int
        {  $$ = create_node(TYPE_SPECIFIER, SIGNED_SHORT); }
    | signed_int
        {  $$ = create_node(TYPE_SPECIFIER, SIGNED_INT); }
    | signed_long_int
        {  $$ = create_node(TYPE_SPECIFIER, SIGNED_LONG); }
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
        {  $$ = create_node(TYPE_SPECIFIER, UNSIGNED_SHORT); }
    | UNSIGNED INT
        {  $$ = create_node(TYPE_SPECIFIER, UNSIGNED_INT); }
    | UNSIGNED LONG INT
        {  $$ = create_node(TYPE_SPECIFIER, UNSIGNED_LONG); }
    ;

character_type_specifier : CHAR
        {  $$ = create_node(TYPE_SPECIFIER, SIGNED_CHAR); }
    | SIGNED CHAR
        {  $$ = create_node(TYPE_SPECIFIER, SIGNED_CHAR); }
    | UNSIGNED CHAR
        {  $$ = create_node(TYPE_SPECIFIER, UNSIGNED_CHAR); }
    ;

void_type_specifier : VOID
    {  $$ = create_node(TYPE_SPECIFIER, VOID); }


abstract_declarator : pointer direct_abstract_declarator
        {
            Node *n = create_node(ABSTRACT_DECLARATOR);
            append_child(n, (Node *) $1, LEFT);
            append_child(n, (Node *) $2, RIGHT);
            $$ = n;
        }
    | pointer
    | direct_abstract_declarator
    ;

pointer : ASTERISK
        {  $$ = create_node(POINTER); }
    | ASTERISK pointer
        {
            Node *n = (Node *) create_node(POINTER);
            n->children[RIGHT] = $2;
            $$ = n;
        }
    ;

direct_abstract_declarator : LEFT_PAREN abstract_declarator RIGHT_PAREN
        {
            Node *n = create_node(PAREN_DIR_ABS_DECL);
            n->children[LEFT] = $2;
            $$ = n;
        }
    | direct_abstract_declarator LEFT_BRACKET conditional_expr RIGHT_BRACKET
        {
            Node *n = create_node(BRACKET_DIR_ABS_DECL);
            n->children[LEFT] = $1;
            n->children[RIGHT] = $3;
            $$ = n;
        }
    | direct_abstract_declarator LEFT_BRACKET RIGHT_BRACKET
        {
            Node *n = create_node(BRACKET_DIR_ABS_DECL);
            n->children[LEFT] = $1;
            $$ = n;
        }
    | LEFT_BRACKET conditional_expr RIGHT_BRACKET
        {
            Node *n = create_node(BRACKET_DIR_ABS_DECL);
            n->children[RIGHT] = $2;
            $$ = n;
        }
    | LEFT_BRACKET RIGHT_BRACKET
        { $$ = create_node(BRACKET_DIR_ABS_DECL); }
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
                                get_token_name(n_type), yylineno);
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
                                get_token_name(n->n_type), yylineno);
            break;
        }
}

void *create_node(enum node_type nt, ...) {
    Node *n = construct_node(nt);
    initialize_children(n);
    va_list ap;
    va_start(ap, nt);
    switch (nt) {
        case PAREN_DIR_ABS_DECL:
        case BRACKET_DIR_ABS_DECL:
        case ABSTRACT_DECLARATOR:
        case TYPE_NAME:
        case POINTER:
            break;
        case TYPE_SPECIFIER:
            n->data.symbols[TYPE] = va_arg(ap, int);
            break;
        /* for identifiers and constants yylval should have been passed in */
        case IDENTIFIER:
            n->data.values.str =
                strdup( ((struct String *) va_arg(ap, YYSTYPE))->str );
            break;
        case NUMBER_CONSTANT:
            n->data.values.num = ((struct Number *) va_arg(ap, YYSTYPE))->value;
            break;
        case CHAR_CONSTANT:
            n->data.values.ch = ((struct Character *) va_arg(ap, YYSTYPE))->c;
            break;
        default:
            handle_parser_error(PE_UNRECOGNIZED_NODE_TYPE, "create_node",
                                yylineno);
            return;
    }
    va_end(ap);

    return (void *) n;
}

void *construct_node(enum node_type nt) {
    Node *n;
    util_emalloc((void **) &n, sizeof(Node));
    n->n_type = nt;
    return n;
}

void initialize_children(Node *n) {
    ChildIndex i;
    for (i = 0; i < MAX_CHILDREN; i++) {
        n->children[i] = NULL;
    }
}

void *create_zero_item_node(enum node_type nt) {
    Node *n = construct_node(nt);
    initialize_children(n);
    return (void *) n;
}

void *create_one_item_node(enum node_type nt, int item1) {
    Node *n = construct_node(nt);
    n->data.symbols[0] = item1;
    initialize_children(n);
    return (void *) n;
}

void append_child(Node *n, Node *child, ChildIndex chidx) {
    n->children[chidx] = child;
}

void *create_binary_expr_node(int op, void *left, void *right) {
    Node *n = create_one_item_node(BINARY_EXPR, op);
    append_child(n, (Node *) left, LEFT);
    append_child(n, (Node *) right, RIGHT);
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
        case TYPE_NAME:
            traverse_node(n->children[LEFT]);
            printf(" ");
            traverse_node(n->children[RIGHT]);
            break;
        case ABSTRACT_DECLARATOR:
            traverse_node(n->children[LEFT]);
            traverse_node(n->children[RIGHT]);
            break;
        case PAREN_DIR_ABS_DECL:
        case BRACKET_DIR_ABS_DECL:
            traverse_direct_abstract_declarator(n);
            break;
        case IDENTIFIER:
        case CHAR_CONSTANT:
        case NUMBER_CONSTANT:
        case STRING_CONSTANT:
            traverse_data_node(n);
            break;
        case BINARY_EXPR:
            traverse_node(n->children[LEFT]);
            printf(" %s ", get_operator_value(n->data.symbols[BINARY_OP]));
            traverse_node(n->children[RIGHT]);
            break;
        default:
            printf("\nwarning: node type not recognized: %d\n", n->n_type);
            break;
    }
}



void traverse_direct_abstract_declarator(Node *n) {
    switch (n->n_type) {
        case PAREN_DIR_ABS_DECL:
            printf("(");
            traverse_node(n->children[LEFT]);
            printf(")");
            break;
        case BRACKET_DIR_ABS_DECL:
            traverse_node(n->children[LEFT]);
            printf("[");
            traverse_node(n->children[RIGHT]);
            printf("]");
            break;
        default:
            break;
    }
}

void pretty_print(Node *n) {
    traverse_node(n);
}

char *get_type_name(enum data_type type) {
    switch (type) {
        case VOID:
            return "void";
        case SIGNED_CHAR:
            return "char";
        case UNSIGNED_CHAR:
            return "unsigned char";
        case SIGNED_SHORT:
            return "short";
        case UNSIGNED_SHORT:
            return "unsigned short";
        case SIGNED_INT:
            return "int";
        case UNSIGNED_INT:
            return "unsigned int";
        case SIGNED_LONG:
            return "long";
        case UNSIGNED_LONG:
            return "unsigned long";
        default:
            return "";
    }
}

char *get_operator_value(int op) {
    switch (op) {
        case ASSIGN:
            return "=";
        case ADD_ASSIGN:
            return "+=";
        case SUBTRACT_ASSIGN:
            return "-=";
        case MULTIPLY_ASSIGN:
            return "*=";
        case DIVIDE_ASSIGN:
            return "/=";
        case REMAINDER_ASSIGN:
            return "%=";
        case BITWISE_LSHIFT_ASSIGN:
            return "<<=";
        case BITWISE_RSHIFT_ASSIGN:
            return ">>=";
        case BITWISE_AND_ASSIGN:
            return "&=";
        case BITWISE_XOR_ASSSIGN:
            return "^=";
        case BITWISE_OR_ASSIGN:
            return "|=";
        case COMMA:
            return ",";
        default:
            printf("op not recognized\n");
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
        case PE_UNRECOGNIZED_NODE_TYPE:
            error(0, 0, "line %d: %s: unrecognized node type", line, data);
            return;
        default:
            return;
    }
}
