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


%%      /*  beginning  of  rules  section  */

translation_unit : top_level_decl
        { traverse_node($1); printf("\n"); }
    | translation_unit top_level_decl
        { traverse_node($2); printf("\n"); }
    ;

top_level_decl : decl
    | function_definition
    ;

decl : type_specifier initialized_declarator_list SEMICOLON
        { $$ = create_node(DECL, $1, $2); }
    ;

function_definition : function_def_specifier compound_statement
    ;

function_def_specifier : declarator
    | type_specifier declarator
    ;

statement : compound_statement
    ;

/*expression_statement
    | labeled_statement
    | compound_statement
    | conditional_statement
    | iterative_statement
    | break_statement
    | continue_statement
    | return_statement
    | goto_statement
    | null_statement 
    ;
*/

compound_statement : LEFT_BRACE RIGHT_BRACE
    | LEFT_BRACE declaration_or_statement_list RIGHT_BRACE
    ;

declaration_or_statement_list : declaration_or_statement
    | declaration_or_statement_list declaration_or_statement
    ;

declaration_or_statement : decl
    | statement
    ;

initialized_declarator_list : declarator
    | initialized_declarator_list COMMA declarator
    ;

declarator : pointer_declarator
    | direct_declarator
    ;

pointer_declarator : pointer direct_declarator
        { $$ = create_node(POINTER_DECLARATOR, $1, $2);  }
    ;

direct_declarator : simple_declarator
    | LEFT_PAREN declarator RIGHT_PAREN
    | function_declarator
    | array_declarator
    ;

simple_declarator : IDENTIFIER
        { $$ = create_node( IDENTIFIER, yylval ); }
    ;

function_declarator : direct_declarator LEFT_PAREN parameter_list RIGHT_PAREN
    ;

parameter_list : parameter_decl
    | parameter_list COMMA parameter_decl
    ;

parameter_decl : type_specifier declarator
    | type_specifier abstract_declarator
    | type_specifier
    ;

array_declarator : direct_declarator LEFT_BRACKET RIGHT_BRACKET
    | direct_declarator LEFT_BRACKET conditional_expr RIGHT_BRACKET
    ;

conditional_expr : logical_or_expr
    | logical_or_expr TERNARY_CONDITIONAL expr COLON conditional_expr
        { $$ = create_node(IF_THEN_ELSE, $1, $3, $5); }
    ;

logical_or_expr : logical_and_expr
    | logical_or_expr LOGICAL_OR logical_and_expr
        { $$ = create_node(BINARY_EXPR, LOGICAL_OR, $1, $3); }
    ;

logical_and_expr : bitwise_or_expr
    | logical_and_expr LOGICAL_AND bitwise_or_expr
        { $$ = create_node(BINARY_EXPR, LOGICAL_AND, $1, $3); }
    ;

bitwise_or_expr : bitwise_xor_expr
    | bitwise_or_expr BITWISE_OR bitwise_xor_expr
        { $$ = create_node(BINARY_EXPR, BITWISE_OR, $1, $3); }
    ;

bitwise_xor_expr : bitwise_and_expr
    | bitwise_xor_expr BITWISE_XOR bitwise_and_expr
        { $$ = create_node(BINARY_EXPR, BITWISE_XOR, $1, $3); }
    ;

bitwise_and_expr : equality_expr
    | bitwise_and_expr AMPERSAND equality_expr
        { $$ = create_node(BINARY_EXPR, AMPERSAND, $1, $3); }
    ;

equality_expr : relational_expr
    | equality_expr EQUAL relational_expr
        { $$ = create_node(BINARY_EXPR, EQUAL, $1, $3); }
    | equality_expr NOT_EQUAL relational_expr
        { $$ = create_node(BINARY_EXPR, NOT_EQUAL, $1, $3); }
    ;

relational_expr : shift_expr
    | relational_expr LESS_THAN shift_expr
        { $$ = create_node(BINARY_EXPR, LESS_THAN, $1, $3); }
    | relational_expr LESS_THAN_EQUAL shift_expr
        { $$ = create_node(BINARY_EXPR, LESS_THAN_EQUAL, $1, $3); }
    | relational_expr GREATER_THAN shift_expr
        { $$ = create_node(BINARY_EXPR, GREATER_THAN, $1, $3); }
    | relational_expr GREATER_THAN_EQUAL shift_expr
        { $$ = create_node(BINARY_EXPR, GREATER_THAN_EQUAL, $1, $3); }
    ;

shift_expr : additive_expr
    | shift_expr BITWISE_LSHIFT additive_expr
        { $$ = create_node(BINARY_EXPR, BITWISE_LSHIFT, $1, $3); }
    | shift_expr BITWISE_RSHIFT additive_expr
        { $$ = create_node(BINARY_EXPR, BITWISE_RSHIFT, $1, $3); }
    ;

additive_expr : multiplicative_expr
    | additive_expr PLUS multiplicative_expr
        { $$ = create_node(BINARY_EXPR, PLUS, $1, $3); }
    | additive_expr MINUS multiplicative_expr
        { $$ = create_node(BINARY_EXPR, MINUS, $1, $3); }
    ;

multiplicative_expr : cast_expr
    | multiplicative_expr ASTERISK cast_expr
        { $$ = create_node(BINARY_EXPR, ASTERISK, $1, $3); }
    | multiplicative_expr DIVIDE cast_expr
        { $$ = create_node(BINARY_EXPR, DIVIDE, $1, $3); }
    | multiplicative_expr REMAINDER cast_expr
        { $$ = create_node(BINARY_EXPR, REMAINDER, $1, $3); }
    ;

cast_expr : unary_expr
    | LEFT_PAREN type_name RIGHT_PAREN cast_expr
        { $$ = create_node(CAST_EXPR, $2, $4); }
    ;

/* unary_expr and children */
unary_expr : postfix_expr
    | MINUS cast_expr
        { $$ = create_node(UNARY_EXPR, MINUS, $2); }
    | PLUS cast_expr
        { $$ = create_node(UNARY_EXPR, PLUS, $2); }
    | LOGICAL_NOT cast_expr
        { $$ = create_node(UNARY_EXPR, LOGICAL_NOT, $2); }
    | BITWISE_NOT cast_expr
        { $$ = create_node(UNARY_EXPR, BITWISE_NOT, $2); }
    | AMPERSAND cast_expr
        { $$ = create_node(UNARY_EXPR, AMPERSAND, $2); }
    | ASTERISK cast_expr
        { $$ = create_node(UNARY_EXPR, ASTERISK, $2); }
    | INCREMENT unary_expr
        { $$ = create_node(PREFIX_EXPR, INCREMENT, $2); }
    | DECREMENT unary_expr
        { $$ = create_node(PREFIX_EXPR, DECREMENT, $2); }
    ;

/* postfix_expr and children */
postfix_expr : primary_expr
    | subscript_expr
    | function_call
    | postfix_expr INCREMENT
        { $$ = create_node(POSTFIX_INCREMENT, INCREMENT, $1); }
    | postfix_expr DECREMENT
        { $$ = create_node(POSTFIX_DECREMENT, DECREMENT, $1); }
    ;

primary_expr : IDENTIFIER
        { $$ = create_node( IDENTIFIER, yylval ); }
    | constant
    | LEFT_PAREN expr RIGHT_PAREN
        { $$ = $2; }
    ;

constant : CHAR_CONSTANT
        { $$ = create_node( CHAR_CONSTANT, yylval ); }
    | STRING_CONSTANT
        { $$ = create_node( STRING_CONSTANT, yylval ); }
    | NUMBER_CONSTANT
        { $$ = create_node( NUMBER_CONSTANT, yylval ); }
    ;

subscript_expr: postfix_expr LEFT_BRACKET expr RIGHT_BRACKET
        { $$ = create_node(SUBSCRIPT_EXPR, $1, $3); }
    ;

function_call : postfix_expr LEFT_PAREN RIGHT_PAREN
        { $$ = create_node(FUNCTION_CALL, $1, NULL); }
    | postfix_expr LEFT_PAREN comma_expr RIGHT_PAREN
        { $$ = create_node(FUNCTION_CALL, $1, $3); }
    ;

expr : comma_expr
    ;

comma_expr : assignment_expr
    | comma_expr COMMA assignment_expr
    { $$ = create_node(BINARY_EXPR, COMMA, $1, $3); }
    ;

assignment_expr : conditional_expr
    | unary_expr ASSIGN assignment_expr
        { $$ = create_node(BINARY_EXPR, ASSIGN, $1, $3); }
    | unary_expr ADD_ASSIGN assignment_expr
        { $$ = create_node(BINARY_EXPR, ADD_ASSIGN, $1, $3); }
    | unary_expr SUBTRACT_ASSIGN assignment_expr
        { $$ = create_node(BINARY_EXPR, SUBTRACT_ASSIGN, $1, $3); }
    | unary_expr MULTIPLY_ASSIGN assignment_expr
        { $$ = create_node(BINARY_EXPR, MULTIPLY_ASSIGN, $1, $3); }
    | unary_expr DIVIDE_ASSIGN assignment_expr
        { $$ = create_node(BINARY_EXPR, DIVIDE_ASSIGN, $1, $3); }
    | unary_expr REMAINDER_ASSIGN assignment_expr
        { $$ = create_node(BINARY_EXPR, REMAINDER_ASSIGN, $1, $3); }
    | unary_expr BITWISE_LSHIFT_ASSIGN assignment_expr
        { $$ = create_node(BINARY_EXPR, BITWISE_LSHIFT_ASSIGN, $1, $3); }
    | unary_expr BITWISE_RSHIFT_ASSIGN assignment_expr
        { $$ = create_node(BINARY_EXPR, BITWISE_RSHIFT_ASSIGN, $1, $3); }
    | unary_expr BITWISE_AND_ASSIGN assignment_expr
        { $$ = create_node(BINARY_EXPR, BITWISE_AND_ASSIGN, $1, $3); }
    | unary_expr BITWISE_XOR_ASSSIGN assignment_expr
        { $$ = create_node(BINARY_EXPR, BITWISE_XOR_ASSSIGN, $1, $3); }
    | unary_expr BITWISE_OR_ASSIGN assignment_expr
        { $$ = create_node(BINARY_EXPR, BITWISE_OR_ASSIGN, $1, $3); }
    ;

/* type name and children */
type_name : type_specifier
    | type_specifier abstract_declarator
        { $$ = create_node(TYPE_NAME, $1, $2); }
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
        { $$ = create_node(ABSTRACT_DECLARATOR, $1, $2);  }
    | pointer
    | direct_abstract_declarator
    ;

pointer : ASTERISK
        {  $$ = create_node(POINTER, NULL); }
    | ASTERISK pointer
        { $$ = create_node(POINTER, $2); }
    ;

direct_abstract_declarator : LEFT_PAREN abstract_declarator RIGHT_PAREN
        { $$ = create_node(PAREN_DIR_ABS_DECL, $2); }
    | direct_abstract_declarator LEFT_BRACKET conditional_expr RIGHT_BRACKET
        { $$ = create_node(BRACKET_DIR_ABS_DECL, $1, $3); }
    | direct_abstract_declarator LEFT_BRACKET RIGHT_BRACKET
        { $$ = create_node(BRACKET_DIR_ABS_DECL, $1, NULL); }
    | LEFT_BRACKET conditional_expr RIGHT_BRACKET
        { $$ = create_node(BRACKET_DIR_ABS_DECL, NULL, $2); }
    | LEFT_BRACKET RIGHT_BRACKET
        { $$ = create_node(BRACKET_DIR_ABS_DECL, NULL, NULL); }
    ;


%%      /*  start  of  programs  */
#include "lex.yy.c"

main() {
  return yyparse();
}

void yyerror(char *s) {
  fprintf(stderr, "%s\n", s);
}

/*
 * create_node
 * Purpose: Create a new node for the parse tree.
 * Parameters:
 *      nt - the node type of the node to be created. The created Node
 *          will have n_type set to nt. Also create_node uses nt
 *          to decide how to parse the rest of the parameters.
 *      ... - variable argument list depending on node type. The pattern is
 *          to first pass data values that will become fields in the created
 *          Node, and then to pass references to the node's children.
 *
 *          Example: for a binary additive expression the call looks like
 *          create_node(BINARY_EXPR, PLUS, left operand, right operand);
 *          This call will set the node's OPERATOR data field to PLUS and its
 *          children to left operand and right operand. Order matters.
 *
 * Returns: A reference to the created node
 * Side effects: Allocates and sets heap storage
 */
void *create_node(enum node_type nt, ...) {
    Node *n = construct_node(nt);
    Node *child1, *child2, *child3;
    initialize_children(n);
    va_list ap;
    va_start(ap, nt);
    switch (nt) {
        case DECL:
            child1 = va_arg(ap, Node *); child2 = va_arg(ap, Node *);
            append_two_children(n, child1, child2);
            break;
        case IF_THEN_ELSE:
            child1 = va_arg(ap, Node *);
            child2 = va_arg(ap, Node *);
            child3 = va_arg(ap, Node *);
            append_three_children(n, child1, child2, child3);
            break;
        case BINARY_EXPR:
            n->data.symbols[OPERATOR] = va_arg(ap, int);
            child1 = va_arg(ap, Node *); child2 = va_arg(ap, Node *);
            append_two_children(n, child1, child2);
            break;
        case POINTER_DECLARATOR:
        case FUNCTION_CALL:
        case CAST_EXPR:
        case TYPE_NAME:
        case ABSTRACT_DECLARATOR:
        case BRACKET_DIR_ABS_DECL:
        case SUBSCRIPT_EXPR:
            child1 = va_arg(ap, Node *); child2 = va_arg(ap, Node *);
            append_two_children(n, child1, child2);
            break;
        case UNARY_EXPR:
        case PREFIX_EXPR:
        case POSTFIX_INCREMENT:
        case POSTFIX_DECREMENT:
            n->data.symbols[OPERATOR] = va_arg(ap, int);
            n->children.unary_op.operand = va_arg(ap, Node *);
            break;
        case TYPE_SPECIFIER:
            n->data.symbols[TYPE] = va_arg(ap, int);
            break;
        case POINTER:
            n->children.ptr.right = va_arg(ap, Node *);
            break;
        case PAREN_DIR_ABS_DECL:
            n->children.dir_abs_decl.abs_decl = va_arg(ap, Node *);
            break;
        /* for identifiers and constants yylval should have been passed in */
        case IDENTIFIER:
            n->data.str =
                strdup( ((struct String *) va_arg(ap, YYSTYPE))->str );
            break;
        case NUMBER_CONSTANT:
            n->data.num = ((struct Number *) va_arg(ap, YYSTYPE))->value;
            break;
        case CHAR_CONSTANT:
            n->data.ch = ((struct Character *) va_arg(ap, YYSTYPE))->c;
            break;
        default:
            handle_parser_error(PE_UNRECOGNIZED_NODE_TYPE, "create_node",
                                yylineno);
            free(n);
            return NULL;
    }
    va_end(ap);

    return (void *) n;
}

void append_two_children(Node *n, Node *child1, Node *child2) {
    switch (n->n_type) {
        case DECL:
            n->children.decl.type_spec = child1;
            n->children.decl.init_decl_ls = child2;
            break;
        case POINTER_DECLARATOR:
            n->children.ptr_decl.ptr = child1;
            n->children.ptr_decl.dir_decl = child2;
            break;
        case BINARY_EXPR:
            n->children.bin_op.left = child1;
            n->children.bin_op.right = child2;
            break;
        case CAST_EXPR:
            n->children.cast_expr.type_name = child1;
            n->children.cast_expr.cast_expr = child2;
            break;
        case SUBSCRIPT_EXPR:
            n->children.subs_expr.pstf_expr = child1;
            n->children.subs_expr.expr = child2;
            break;
        case FUNCTION_CALL:
            n->children.function_call.pstf_expr = child1;
            n->children.function_call.expr_list = child2;
            break;
        case ABSTRACT_DECLARATOR:
            n->children.abs_decl.ptr = child1;
            n->children.abs_decl.dir_abs_decl = child2;
            break;
        case BRACKET_DIR_ABS_DECL:
            n->children.dir_abs_decl.dir_abs_decl = (Node *) child1;
            n->children.dir_abs_decl.cond_expr = (Node *) child2;
            break;
        case TYPE_NAME:
            n->children.type_name.type_spec = child1;
            n->children.type_name.abs_decl = child2;
            break;
        default:
            #ifdef DEBUG
                printf("nt %d ", n->n_type);
            #endif
            handle_parser_error(PE_UNRECOGNIZED_NODE_TYPE,
                                "append_two_children", yylineno);
    }
}

void append_three_children(Node *n, Node *child1, Node *child2, Node *child3) {
    switch (n->n_type) {
        case IF_THEN_ELSE:
            n->children.if_then_else.cond = child1;
            n->children.if_then_else.val_if_true = child2;
            n->children.if_then_else.val_if_false = child3;
            break;
        default:
            #ifdef DEBUG
                printf("nt %d", n->n_type);
            #endif
            handle_parser_error(PE_UNRECOGNIZED_NODE_TYPE,
                                "append_three_children", yylineno);
    }
}

void initialize_children(Node *n) {
    switch (n->n_type) {
        case DECL:
            n->children.decl.type_spec = NULL;
            n->children.decl.init_decl_ls = NULL;
            break;
        case POINTER_DECLARATOR:
            n->children.ptr_decl.ptr = NULL;
            n->children.ptr_decl.dir_decl = NULL;
            break;
        case IF_THEN_ELSE:
            n->children.if_then_else.cond = NULL;
            n->children.if_then_else.val_if_true = NULL;
            n->children.if_then_else.val_if_false = NULL;
        case BINARY_EXPR:
            n->children.bin_op.left = NULL;
            n->children.bin_op.right = NULL;
            break;
        case CAST_EXPR:
            n->children.cast_expr.type_name = NULL;
            n->children.cast_expr.cast_expr = NULL;
            break;
        case SUBSCRIPT_EXPR:
            n->children.subs_expr.pstf_expr = NULL;
            n->children.subs_expr.expr = NULL;
            break;
        case FUNCTION_CALL:
            n->children.function_call.pstf_expr = NULL;
            n->children.function_call.expr_list = NULL;
            break;
        case UNARY_EXPR:
        case PREFIX_EXPR:
        case POSTFIX_INCREMENT:
        case POSTFIX_DECREMENT:
            n->children.unary_op.operand = NULL;
            break;
        case POINTER:
            n->children.ptr.right = NULL;
            break;
        case TYPE_NAME:
            n->children.type_name.type_spec = NULL;
            n->children.type_name.abs_decl = NULL;
            break;
        case ABSTRACT_DECLARATOR:
            n->children.abs_decl.ptr = NULL;
            n->children.abs_decl.dir_abs_decl = NULL;
            break;
        case PAREN_DIR_ABS_DECL:
        case BRACKET_DIR_ABS_DECL:
            n->children.dir_abs_decl.abs_decl = NULL;
            n->children.dir_abs_decl.dir_abs_decl = NULL;
            n->children.dir_abs_decl.cond_expr = NULL;
            break;
        case TYPE_SPECIFIER:
        case IDENTIFIER:
        case CHAR_CONSTANT:
        case NUMBER_CONSTANT:
        case STRING_CONSTANT:
            break;
        default:
            handle_parser_error(PE_UNRECOGNIZED_NODE_TYPE,"initialize_children",
                                yylineno);
    }
}

void *construct_node(enum node_type nt) {
    Node *n;
    util_emalloc((void **) &n, sizeof(Node));
    n->n_type = nt;
    return n;
}


void traverse_node(void *np) {
    Node *n = (Node *) np;
    if (n == NULL) {
        return;
    }
    switch (n->n_type) {
        case DECL:
            traverse_node(n->children.decl.type_spec);
            printf(" ");
            traverse_node(n->children.decl.init_decl_ls);
            break;
        case POINTER_DECLARATOR:
            printf("(");
            traverse_node(n->children.ptr_decl.ptr);
            traverse_node(n->children.ptr_decl.dir_decl);
            printf(")");
            break;
        case IF_THEN_ELSE:
            traverse_node(n->children.if_then_else.cond);
            printf(" ? ");
            traverse_node(n->children.if_then_else.val_if_true);
            printf(" : ");
            traverse_node(n->children.if_then_else.val_if_false);
            break;
        case CAST_EXPR:
            printf("(");
            traverse_node(n->children.cast_expr.type_name);
            printf(") ");
            traverse_node(n->children.cast_expr.cast_expr);
            break;
        case BINARY_EXPR:
            traverse_node(n->children.bin_op.left);
            printf(" %s ", get_operator_value(n->data.symbols[OPERATOR]));
            traverse_node(n->children.bin_op.right);
            break;
        case TYPE_NAME:
            traverse_node(n->children.type_name.type_spec);
            printf(" ");
            traverse_node(n->children.type_name.abs_decl);
            break;
        case TYPE_SPECIFIER:
            printf("%s", get_type_name(n->data.symbols[TYPE]));
            break;
        case ABSTRACT_DECLARATOR:
            traverse_node(n->children.abs_decl.ptr);
            traverse_node(n->children.abs_decl.dir_abs_decl);
            break;
        case POINTER:
            printf("*");
            traverse_node((void *) n->children.ptr.right);
            break;
        case PAREN_DIR_ABS_DECL:
        case BRACKET_DIR_ABS_DECL:
            traverse_direct_abstract_declarator(n);
            break;
        case SUBSCRIPT_EXPR:
            traverse_node(n->children.subs_expr.pstf_expr);
            printf("[");
            traverse_node(n->children.subs_expr.expr);
            printf("]");
            break;
        case FUNCTION_CALL:
            traverse_node(n->children.function_call.pstf_expr);
            printf("(");
            traverse_node(n->children.function_call.expr_list);
            printf(")");
            break;
        case UNARY_EXPR:
        case PREFIX_EXPR:
            printf("%s", get_operator_value(n->data.symbols[OPERATOR]));
            traverse_node(n->children.unary_op.operand);
            break;
        case POSTFIX_INCREMENT:
        case POSTFIX_DECREMENT:
            traverse_node(n->children.unary_op.operand);
            printf("%s", get_operator_value(n->data.symbols[OPERATOR]));
            break;
        case IDENTIFIER:
        case CHAR_CONSTANT:
        case NUMBER_CONSTANT:
        case STRING_CONSTANT:
            traverse_data_node(n);
            break;
        default:
            printf("\nwarning: node type not recognized: %d\n", n->n_type);
            break;
    }
}

void traverse_data_node(void *np) {
    Node *n = (Node *) np;
    switch (n->n_type) {
        case IDENTIFIER:
            printf("%s", n->data.str);
            break;
        case STRING_CONSTANT:
            /* TODO: replace special characters, e.g. replace newline with \n */
            printf("\"%s\"", n->data.str);
            break;
        case NUMBER_CONSTANT:
            printf("%d", n->data.num);
            break;
        case CHAR_CONSTANT:
            /* TODO: replace special characters, e.g. replace null with \0 */
            printf("'%c'", n->data.ch);
            break;
        default:
            handle_parser_error(PE_INVALID_DATA_TYPE,
                                get_token_name(n->n_type), yylineno);
            break;
        }
}

void traverse_direct_abstract_declarator(Node *n) {
    switch (n->n_type) {
        case PAREN_DIR_ABS_DECL:
            printf("(");
            traverse_node(n->children.dir_abs_decl.abs_decl);
            printf(")");
            break;
        case BRACKET_DIR_ABS_DECL:
            traverse_node(n->children.dir_abs_decl.dir_abs_decl);
            printf("[");
            traverse_node(n->children.dir_abs_decl.cond_expr);
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
        case INCREMENT:
            return "++";
        case DECREMENT:
            return "--";
        case MINUS:
            return "-";
        case PLUS:
            return "+";
        case LOGICAL_NOT:
            return "!";
        case BITWISE_NOT:
            return "~";
        case AMPERSAND:
            return "&";
        case ASTERISK:
            return "*";
        case LOGICAL_OR:
            return "||";
        case LOGICAL_AND:
            return "&&";
        case BITWISE_OR:
            return "|";
        case BITWISE_XOR:
            return "^";
        case EQUAL:
            return "==";
        case NOT_EQUAL:
            return "!=";
        case LESS_THAN:
            return "<";
        case LESS_THAN_EQUAL:
            return "<=";
        case GREATER_THAN:
            return ">";
        case GREATER_THAN_EQUAL:
            return ">=";
        case BITWISE_LSHIFT:
            return "<<";
        case BITWISE_RSHIFT:
            return ">>";
        case DIVIDE:
            return "/";
        case REMAINDER:
            return "%";
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
