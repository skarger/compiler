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

translation_unit : conditional_expr SEMICOLON
        { traverse_node($1); printf("\n"); }
    | translation_unit conditional_expr SEMICOLON
        { traverse_node($2); printf("\n"); }
    ;

conditional_expr : logical_or_expr
    | logical_or_expr TERNARY_CONDITIONAL expr COLON conditional_expr
    ;

logical_or_expr : logical_and_expr
    | logical_or_expr LOGICAL_OR logical_and_expr
    ;

logical_and_expr : bitwise_or_expr
    | logical_and_expr LOGICAL_AND bitwise_or_expr
    ;

bitwise_or_expr : bitwise_xor_expr
    | bitwise_or_expr BITWISE_OR bitwise_xor_expr
    ;

bitwise_xor_expr : bitwise_and_expr
    | bitwise_xor_expr BITWISE_XOR bitwise_and_expr
    ;

bitwise_and_expr : equality_expr
    | bitwise_and_expr AMPERSAND equality_expr
    ;

equality_expr : relational_expr
    | equality_expr EQUAL relational_expr
    | equality_expr NOT_EQUAL relational_expr
    ;

relational_expr : shift_expr
    | relational_expr LESS_THAN shift_expr
    | relational_expr LESS_THAN_EQUAL shift_expr
    | relational_expr GREATER_THAN shift_expr
    | relational_expr GREATER_THAN_EQUAL shift_expr
    ;

shift_expr : additive_expr
    | shift_expr BITWISE_LSHIFT additive_expr
    | shift_expr BITWISE_RSHIFT additive_expr
    ;

additive_expr : multiplicative_expr
    | additive_expr PLUS multiplicative_expr
    | additive_expr MINUS multiplicative_expr
    ;

multiplicative_expr : cast_expr
    | multiplicative_expr ASTERISK cast_expr
    | multiplicative_expr DIVIDE cast_expr
    | multiplicative_expr REMAINDER cast_expr
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
        { Node *n = create_node(PAREN_DIR_ABS_DECL, $2); }
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
    Node *child1, *child2;
    initialize_children(n);
    va_list ap;
    va_start(ap, nt);
    switch (nt) {
        case BINARY_EXPR:
            n->data.symbols[OPERATOR] = va_arg(ap, int);
            child1 = va_arg(ap, Node *); child2 = va_arg(ap, Node *);
            append_two_children(n, child1, child2);
            break;
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

void *append_two_children(Node *n, Node *child1, Node *child2) {
    switch (n->n_type) {
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
                printf("nt %d", n->n_type);
            #endif
            handle_parser_error(PE_UNRECOGNIZED_NODE_TYPE,
                                "append_two_children", yylineno);
    }
    return (void *) n;
}

void initialize_children(Node *n) {
    switch (n->n_type) {
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
