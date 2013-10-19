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
        { pretty_print($1); printf("\n"); }
    | translation_unit top_level_decl
        { pretty_print($2); printf("\n"); }
    ;

top_level_decl : decl
    | function_definition
    ;

function_definition : function_def_specifier compound_statement
        { $$ = create_node(FUNCTION_DEFINITION, $1, $2); }
    ;

function_def_specifier : declarator
        {
            yyerror("return type missing from function specifier"); yyerrok;
            $$ = create_node(FUNCTION_DEF_SPEC, $1, NULL);
        }
    | type_specifier function_declarator
        { $$ = create_node(FUNCTION_DEF_SPEC, $1, $2); }
    | type_specifier parenthesized_declarator
        { $$ = create_node(FUNCTION_DEF_SPEC, $1, $2); }
    | type_specifier non_function_declarator
        {
            yyerror("invalid function declarator"); yyerrok;
            $$ = create_node(FUNCTION_DEF_SPEC, $1, $2);
        }
    ;

/* decl and close children */
decl : type_specifier initialized_declarator_list SEMICOLON
        {
            Node *n = $1;
            if (n->data.symbols[TYPE_SPEC] == VOID) {
                { yyerror("void declaration not permitted"); yyerrok; }
            }
            $$ = create_node(DECL, $1, $2);
        }
    ;

initialized_declarator_list : initialized_declarator
    | initialized_declarator_list COMMA initialized_declarator
        { $$ = create_node(INIT_DECL_LIST, $1, $3); }
    ;

/* initializer productions created for error checking */
initialized_declarator : declarator
    | declarator ASSIGN initializer
        { yyerror("initializers are not allowed in a definition"); yyerrok; }
    ;

initializer : assignment_expr
    | LEFT_BRACE initializer_list COMMA RIGHT_BRACE
    | LEFT_BRACE initializer_list RIGHT_BRACE
    ;

initializer_list : initializer
    | initializer_list COMMA initializer
    ;
/* end initializer productions */


declarator : pointer_declarator
    | direct_declarator
    ;

pointer_declarator : pointer direct_declarator
        { $$ = create_node(POINTER_DECLARATOR, $1, $2);  }
    ;

direct_declarator : simple_declarator
    | parenthesized_declarator
    | function_declarator
    | array_declarator
    ;

simple_declarator : identifier
        { set_node_type($1, SIMPLE_DECLARATOR); $$ = $1; }
    ;

identifier : IDENTIFIER
        { $$ = create_node( IDENTIFIER, yylval ); }
    ;

parenthesized_declarator : LEFT_PAREN declarator RIGHT_PAREN
        { $$ = $2; }
    ;

function_declarator : direct_declarator LEFT_PAREN parameter_list RIGHT_PAREN
        { $$ = create_node(FUNCTION_DECLARATOR, $1, $3); }
    | direct_declarator LEFT_PAREN RIGHT_PAREN
        {
            yyerror("function must have a parameter list, even if it is (void)"); yyerrok;
            $$ = create_node(FUNCTION_DECLARATOR, NULL, NULL);
        }
    ;

/* non_function_declarator exists for error checking */
non_function_declarator : simple_declarator
    | array_declarator
    ;

/* void may only appear by itself in a parameter list, which is why it
   is part of its production rather than being part of parameter_decl */
parameter_list : void_type_specifier
    | parameter_decl
    | parameter_list COMMA parameter_decl
        { $$ = create_node(PARAMETER_LIST, $1, $3); }
    ;

parameter_decl : integer_type_specifier declarator
        { $$ = create_node(PARAMETER_DECL, $1, $2); }
    | integer_type_specifier abstract_declarator
        { $$ = create_node(PARAMETER_DECL, $1, $2); }
    | integer_type_specifier
    | void_type_specifier declarator
        {
            yyerror("void may not appear with any other function parameters"); yyerrok;
            $$ = create_node(PARAMETER_DECL, $1, $2);
        }
    | void_type_specifier abstract_declarator
        {
            yyerror("void may not appear with any other function parameters"); yyerrok;
            $$ = create_node(PARAMETER_DECL, $1, $2);
        }
    ;

array_declarator : direct_declarator LEFT_BRACKET RIGHT_BRACKET
        { $$ = create_node(ARRAY_DECLARATOR, $1, NULL); }
    | direct_declarator LEFT_BRACKET conditional_expr RIGHT_BRACKET
        { $$ = create_node(ARRAY_DECLARATOR, $1, $3); }
    ;

/*
 * statement and close children
 * statement, matched_statement, and open_statement productions
 * paraphrased from Aho, Lam, Sethi, Ullman, page 212.
 */
statement : matched_statement
    | open_statement
    ;

matched_statement : IF LEFT_PAREN expr RIGHT_PAREN matched_statement ELSE matched_statement
        { $$ = create_node(IF_THEN_ELSE, $3, $5, $7); }
    | other_matched_statement
    ;

open_statement : IF LEFT_PAREN expr RIGHT_PAREN statement
        { $$ = create_node(IF_THEN, $3, $5); }
    | IF LEFT_PAREN expr RIGHT_PAREN matched_statement ELSE open_statement
        { $$ = create_node(IF_THEN_ELSE, $3, $5, $7); }
    | other_open_statement
    ;

other_matched_statement : expr SEMICOLON
        { $$ = create_node(EXPRESSION_STATEMENT, $1); }
    | compound_statement
    | do_statement
    | BREAK SEMICOLON
        { $$ = create_node(BREAK_STATEMENT); }
    | CONTINUE SEMICOLON
        { $$ = create_node(CONTINUE_STATEMENT); }
    | return_statement
    | GOTO identifier SEMICOLON
        { $$ = create_node(GOTO_STATEMENT, $2); }
    | SEMICOLON
        { $$ = create_node(NULL_STATEMENT); }
    ;

other_open_statement: labeled_statement
    | while_statement
    | for_statement
    ;

labeled_statement : identifier COLON statement
        { create_node(LABELED_STATEMENT, $1, $3); }
    ;

while_statement : WHILE LEFT_PAREN expr RIGHT_PAREN statement
        { $$ = create_node(WHILE_STATEMENT, $3, $5); }
    ;

do_statement : DO statement WHILE LEFT_PAREN expr RIGHT_PAREN SEMICOLON
        { $$ = create_node(DO_STATEMENT, $2, $5); }
    ;

for_statement : FOR LEFT_PAREN expr SEMICOLON expr SEMICOLON expr RIGHT_PAREN statement
        { $$ = create_node(FOR_STATEMENT, $3, $5, $7, $9); }
    ;

return_statement : RETURN SEMICOLON
        { $$ = create_node(RETURN_STATEMENT, NULL); }
    | RETURN expr SEMICOLON
        { $$ = create_node(RETURN_STATEMENT, $2); }
    ;

compound_statement : LEFT_BRACE RIGHT_BRACE
        { $$ = create_node(COMPOUND_STATEMENT, NULL); }
    | LEFT_BRACE declaration_or_statement_list RIGHT_BRACE
        { $$ = create_node(COMPOUND_STATEMENT, $2); }
    ;

declaration_or_statement_list : declaration_or_statement
    | declaration_or_statement_list declaration_or_statement
        { $$ = create_node(DECL_OR_STMT_LIST, $1, $2); }
    ;

declaration_or_statement : decl
    | statement
    ;

/* conditional_expr and close children */
conditional_expr : logical_or_expr
    | logical_or_expr TERNARY_CONDITIONAL expr COLON conditional_expr
        { $$ = create_node(CONDITIONAL_EXPR, $1, $3, $5); }
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
        { $$ = create_node(POSTFIX_EXPR, INCREMENT, $1); }
    | postfix_expr DECREMENT
        { $$ = create_node(POSTFIX_EXPR, DECREMENT, $1); }
    ;

primary_expr : identifier
        { set_node_type($1, IDENTIFIER_EXPR); $$ = $1; }
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
type_name : integer_type_specifier
        { $$ = create_node(TYPE_NAME, $1, NULL); }
    | integer_type_specifier abstract_declarator
        { $$ = create_node(TYPE_NAME, $1, $2); }
    | void_type_specifier
        {
            yyerror("void may not be used in a cast expression"); yyerrok;
            $$ = create_node(TYPE_NAME, $1, NULL);
        }
    | void_type_specifier abstract_declarator
        {
            yyerror("void may not be used in a cast expression"); yyerrok;
            $$ = create_node(TYPE_NAME, $1, $2);
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

/* abstract_declarator and close children */
abstract_declarator : pointer
    | pointer direct_abstract_declarator
        { $$ = create_node(ABSTRACT_DECLARATOR, $1, $2); }
    | direct_abstract_declarator
    ;

pointer : ASTERISK
        {  $$ = create_node(POINTER, NULL); }
    | ASTERISK pointer
        { $$ = create_node(POINTER, $2); }
    ;

direct_abstract_declarator : LEFT_PAREN abstract_declarator RIGHT_PAREN
        { $$ = $2; }
    | direct_abstract_declarator LEFT_BRACKET conditional_expr RIGHT_BRACKET
        { $$ = create_node(DIR_ABS_DECL, $1, $3); }
    | direct_abstract_declarator LEFT_BRACKET RIGHT_BRACKET
        { $$ = create_node(DIR_ABS_DECL, $1, NULL); }
    | LEFT_BRACKET conditional_expr RIGHT_BRACKET
        { $$ = create_node(DIR_ABS_DECL, NULL, $2); }
    | LEFT_BRACKET RIGHT_BRACKET
        { $$ = create_node(DIR_ABS_DECL, NULL, NULL); }
    ;


%%      /*  start  of  programs  */
#include "lex.yy.c"

static FILE *output;
int main(int argc, char *argv[]) {
    extern FILE *yyin;
    FILE *input;

    int rv;

    /* Figure out whether we're using stdin/stdout or file in/file out. */
    if (argc < 2 || !strcmp("-", argv[1])) {
        input = stdin;
    } else {
        input = fopen(argv[1], "r");
    }

    if (argc < 3 || !strcmp("-", argv[2])) {
        output = stdout;
    } else {
        output = fopen(argv[2], "w");
    }

    yyin = input;

    rv = yyparse();

    if (output != stdout) {
        fclose(output);
    }
    if (input != stdin) {
        fclose(input);
    }

    return rv;
}

void yyerror(char *s) {
  fprintf(stderr, "error: line %d: %s\n", yylineno, s);
}

/* Node construction and setter functions */

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
    Node *child1, *child2, *child3, *child4;
    initialize_children(n);
    va_list ap;
    va_start(ap, nt);

    if (has_operator(nt)) {
        set_operator(n, va_arg(ap, int));
    }

    if (nt == TYPE_SPECIFIER) {
        set_type(n, va_arg(ap, int));
    }

    if (has_literal_data(nt)) {
        set_literal_data(n, va_arg(ap, YYSTYPE));
    }

    int num_children = number_of_children(nt);
    switch (num_children) {
        case 1:
            child1 = va_arg(ap, Node *);
            append_children(n, 1, child1);
            break;
        case 2:
            child1 = va_arg(ap, Node *); child2 = va_arg(ap, Node *);
            append_children(n, 2, child1, child2);
            break;
        case 3:
            child1 = va_arg(ap, Node *);
            child2 = va_arg(ap, Node *);
            child3 = va_arg(ap, Node *);
            append_children(n, 3, child1, child2, child3);
            break;
        case 4:
            child1 = va_arg(ap, Node *);
            child2 = va_arg(ap, Node *);
            child3 = va_arg(ap, Node *);
            child4 = va_arg(ap, Node *);
            append_children(n, 4, child1, child2, child3, child4);
            break;
        default:
            break;
    }
    va_end(ap);

    return (void *) n;
}


/* helpers for create_node */
int has_operator(enum node_type nt) {
    switch (nt) {
        case BINARY_EXPR:
        case UNARY_EXPR:
        case PREFIX_EXPR:
        case POSTFIX_EXPR:
            return 1;
        default:
            return 0;
    }
}

int has_literal_data(enum node_type nt) {
    switch (nt) {
        case SIMPLE_DECLARATOR:
        case IDENTIFIER_EXPR:
        case IDENTIFIER:
        case NUMBER_CONSTANT:
        case CHAR_CONSTANT:
        case STRING_CONSTANT:
            return 1;
        default:
            return 0;
    }
}

void set_operator(Node *n, int op) {
    n->data.symbols[OPERATOR] = op;
}

void set_type(Node *n, int type_spec) {
    n->data.symbols[TYPE_SPEC] = type_spec;
}

void set_literal_data(Node *n, YYSTYPE data) {
    switch (n->n_type) {
        /* passed in data is yylval */
        case SIMPLE_DECLARATOR:
        case IDENTIFIER_EXPR:
        case IDENTIFIER:
            n->data.str =
                strdup( ((struct String *) data)->str );
            break;
        case NUMBER_CONSTANT:
            n->data.num = ((struct Number *) data)->value;
            break;
        case CHAR_CONSTANT:
            n->data.ch = ((struct Character *) data)->c;
            break;
        case STRING_CONSTANT:
            n->data.str = ((struct String *) data)->str;
            break;
        default:
            handle_parser_error(PE_UNRECOGNIZED_NODE_TYPE,
                                "set_literal_data",
                                yylineno);
    }
}

int number_of_children(enum node_type nt) {
    switch (nt) {
        case BREAK_STATEMENT:
        case CONTINUE_STATEMENT:
        case NULL_STATEMENT:
            return 0;
        case EXPRESSION_STATEMENT:
        case COMPOUND_STATEMENT:
        case RETURN_STATEMENT:
        case GOTO_STATEMENT:
        case POINTER:
        case UNARY_EXPR:
        case PREFIX_EXPR:
        case POSTFIX_EXPR:
            return 1;
        case DECL_OR_STMT_LIST:
        case INIT_DECL_LIST:
        case FUNCTION_DEFINITION:
        case FUNCTION_DEF_SPEC:
        case DECL:
        case PARAMETER_DECL:
        case PARAMETER_LIST:
        case FUNCTION_DECLARATOR:
        case ARRAY_DECLARATOR:
        case IF_THEN:
        case LABELED_STATEMENT:
        case WHILE_STATEMENT:
        case DO_STATEMENT:
        case POINTER_DECLARATOR:
        case FUNCTION_CALL:
        case CAST_EXPR:
        case TYPE_NAME:
        case ABSTRACT_DECLARATOR:
        case DIR_ABS_DECL:
        case SUBSCRIPT_EXPR:
        case BINARY_EXPR:
            return 2;
        case IF_THEN_ELSE:
        case CONDITIONAL_EXPR:
            return 3;
        case FOR_STATEMENT:
            return 4;
        default:
            handle_parser_error(PE_UNRECOGNIZED_NODE_TYPE,
                                "number_of_children",
                                yylineno);
            return -1;
    }
}


/*
 * append_children
 * Purpose: Given a node, append child nodes to it.
 * Parameters:
 *  n            Node * The node to which this function appends the children.
 *  num_children int    The number of children to append.
 *  Optional arguments representing nodes to append, in order:
 *  Node * The first child node to append.
 *  Node * The second child node to append.
 *  Node * The third child node to append.
 *  Node * The fourth child node to append.
 * Returns: None
 * Side-effects: None
 */
void append_children(Node *n, int num_children, ...) {
    Node *child1, *child2, *child3, *child4;
    va_list ap;
    va_start(ap, num_children);
    if (num_children >= 1) {
        child1 = va_arg(ap, Node *);
        n->children.child1 = child1;
    }
    if (num_children >= 2) {
        child2 = va_arg(ap, Node *);
        n->children.child2 = child2;
    }
    if (num_children >= 3) {
        child3 = va_arg(ap, Node *);
        n->children.child3 = child3;
    }
    if (num_children >= 4) {
        child4 = va_arg(ap, Node *);
        n->children.child4 = child4;
    }
    va_end(ap);
}

/*
 * initialize_children 
 * Purpose: Given a node, initialize its children to NULL.
 * Parameters:
 *  n       Node * The node whose children should be initialized.
 * Returns: None
 * Side-effects: None
 */
void initialize_children(Node *n) {
    n->children.child1 = NULL;
    n->children.child2 = NULL;
    n->children.child3 = NULL;
    n->children.child4 = NULL;
}

/*
 * construct_node
 * Purpose: Construct a new node and set its type to the passed type.
 * Parameters:
 *  nt       enum node_type The type of node to construct.
 * Returns: A void pointer to the constructed node
 * Side-effects: Allocates heap memory
 */
void *construct_node(enum node_type nt) {
    Node *n;
    util_emalloc((void **) &n, sizeof(Node));
    n->n_type = nt;
    return n;
}



void set_node_type(Node *n, enum node_type nt) {
    n->n_type = nt;
}




/* Printing and Tree Traversal Functions */

/*
 * pretty_print
 * Purpose: Traverse the parse tree rooted at the passed node and print
 *          out C syntax appropriately.
 * Parameters:
 *  n       Node * The node to start traversing from, generally a top_level_decl
 * Returns: None
 * Side-effects: None
 */
void pretty_print(Node *n) {
    traverse_node(n);
}

/*
 * traverse_node
 * Purpose: Workhorse function for pretty_print. Does the actually traversal.
 * Parameters:
 *  np      void * The node to start traversing from. Recursively traverses
 *          the children of np.
 * Returns: None
 * Side-effects: None
 */
void traverse_node(void *np) {
    Node *n = (Node *) np;
    if (n == NULL) {
        return;
    }
    #ifdef DEBUG
        printf("traverse_node: node type: %d\n", n->n_type);
    #endif
    if (parenthesize(n->n_type)) {
        fprintf(output, "(");
    }

    switch (n->n_type) {

        case DIR_ABS_DECL:
            traverse_node(n->children.child1);
            fprintf(output, "[");
            traverse_node(n->children.child2);
            fprintf(output, "]");
            break;
        case ABSTRACT_DECLARATOR:
        case POINTER_DECLARATOR:
        case FUNCTION_DEFINITION:
            traverse_node(n->children.child1);
            traverse_node(n->children.child2);
            break;
        case FUNCTION_DEF_SPEC:
        case PARAMETER_DECL:
        case CAST_EXPR:
        case TYPE_NAME:
            traverse_node(n->children.child1);
            fprintf(output, " ");
            traverse_node(n->children.child2);
            break;
        case DECL_OR_STMT_LIST:
            traverse_node(n->children.child1);
            fprintf(output, "\n");
            traverse_node(n->children.child2);
            break;
        case PARAMETER_LIST:
        case INIT_DECL_LIST:
            traverse_node(n->children.child1);
            fprintf(output, ", ");
            traverse_node(n->children.child2);
            break;
        case DECL:
            traverse_node(n->children.child1);
            fprintf(output, " ");
            traverse_node(n->children.child2);
            fprintf(output, ";");
            break;
        case FUNCTION_DECLARATOR:
            traverse_node(n->children.child1);
            fprintf(output, "(");
            traverse_node(n->children.child2);
            fprintf(output, ")");
            break;
        case ARRAY_DECLARATOR:
            traverse_node(n->children.child1);
            fprintf(output, "[");
            traverse_node(n->children.child2);
            fprintf(output, "]");
            break;
        case EXPRESSION_STATEMENT:
            traverse_node(n->children.child1);
            fprintf(output, ";");
            break;
        case LABELED_STATEMENT:
            traverse_node(n->children.child1);
            fprintf(output, " : ");
            traverse_node(n->children.child2);
            break;
        case COMPOUND_STATEMENT:
            fprintf(output, "\n{\n");
            traverse_node(n->children.child1);
            fprintf(output, "\n}\n");
            break;
        case IF_THEN:
        case IF_THEN_ELSE:
            traverse_conditional_statement(n);
            break;
        case WHILE_STATEMENT:
        case DO_STATEMENT:
        case FOR_STATEMENT:
            traverse_iterative_statement(n);
            break;
        case BREAK_STATEMENT:
            fprintf(output, "break;");
            break;
        case CONTINUE_STATEMENT:
            fprintf(output, "continue;");
            break;
        case RETURN_STATEMENT:
            fprintf(output, "return ");
            traverse_node(n->children.child1);
            fprintf(output, ";");
            break;
        case GOTO_STATEMENT:
            fprintf(output, "goto ");
            traverse_node(n->children.child1);
            fprintf(output, ";");
            break;
        case NULL_STATEMENT:
            fprintf(output, ";");
            break;
        case CONDITIONAL_EXPR:
            traverse_node(n->children.child1);
            fprintf(output, " ? ");
            traverse_node(n->children.child2);
            fprintf(output, " : ");
            traverse_node(n->children.child3);
            break;
        case BINARY_EXPR:
            traverse_node(n->children.child1);
            fprintf(output, " %s ", get_operator_value(n->data.symbols[OPERATOR]));
            traverse_node(n->children.child2);
            break;
        case TYPE_SPECIFIER:
            fprintf(output, "%s", get_type_spec(n->data.symbols[TYPE_SPEC]));
            break;
        case POINTER:
            print_pointers(n);
            break;
        case SUBSCRIPT_EXPR:
            traverse_node(n->children.child1);
            fprintf(output, "[");
            traverse_node(n->children.child2);
            fprintf(output, "]");
            break;
        case FUNCTION_CALL:
            traverse_node(n->children.child1);
            fprintf(output, "(");
            traverse_node(n->children.child2);
            fprintf(output, ")");
            break;
        case UNARY_EXPR:
        case PREFIX_EXPR:
            fprintf(output, "%s", get_operator_value(n->data.symbols[OPERATOR]));
            traverse_node(n->children.child1);
            break;
        case POSTFIX_EXPR:
            traverse_node(n->children.child1);
            fprintf(output, "%s", get_operator_value(n->data.symbols[OPERATOR]));
            break;
        case SIMPLE_DECLARATOR:
        case IDENTIFIER_EXPR:
        case IDENTIFIER:
        case CHAR_CONSTANT:
        case NUMBER_CONSTANT:
        case STRING_CONSTANT:
            print_data_node(n);
            break;
        default:
            handle_parser_error(PE_UNRECOGNIZED_NODE_TYPE,"traverse_node",
                                yylineno);
            break;
    }

    if (parenthesize(n->n_type)) {
        fprintf(output, ")");
    }
}

int parenthesize(enum node_type nt) {

    switch (nt) {
        /* declarator */
        case POINTER_DECLARATOR:
        case SIMPLE_DECLARATOR:
        case FUNCTION_DECLARATOR:
        case ARRAY_DECLARATOR:

        /* expr */
        case CONDITIONAL_EXPR:
        case BINARY_EXPR:
        case CAST_EXPR:
        case TYPE_NAME:
        case UNARY_EXPR:
        case PREFIX_EXPR:
        case POSTFIX_EXPR:
        case IDENTIFIER_EXPR:
        case CHAR_CONSTANT:
        case STRING_CONSTANT:
        case NUMBER_CONSTANT:
        case SUBSCRIPT_EXPR:
        case FUNCTION_CALL:

            return 1;
        default:
            return 0;
    }
}

/*
 * traverse_iterative_statement
 * Purpose: Helper function for traverse_node.
 *          Traverses iterative statements.
 * Parameters:
 *  np      void * The node to start traversing from. Recursively traverses
 *          the children of np.
 * Returns: None
 * Side-effects: None
 */
void traverse_iterative_statement(void *np) {
    Node *n = (Node *) np;
    if (n == NULL) {
        return;
    }
    switch (n->n_type) {
        case WHILE_STATEMENT:
            fprintf(output, "while ( ");
            traverse_node(n->children.child1);
            fprintf(output, " ) ");
            traverse_node(n->children.child2);
            break;
        case DO_STATEMENT:
            fprintf(output, "do ");
            traverse_node(n->children.child1);
            fprintf(output, " while ( ");
            traverse_node(n->children.child2);
            fprintf(output, " );");
            break;
        case FOR_STATEMENT:
            fprintf(output, "for ( ");
            traverse_node(n->children.child1);
            fprintf(output, "; ");
            traverse_node(n->children.child2);
            fprintf(output, "; ");
            traverse_node(n->children.child3);
            fprintf(output, " ) ");
            traverse_node(n->children.child4);
            break;
        default:
            handle_parser_error(PE_UNRECOGNIZED_NODE_TYPE,
                                "traverse_iterative_statement",
                                yylineno);
            break;

    }
}

/*
 * traverse_conditional_statement
 * Purpose: Helper function for traverse_node.
 *          Traverses conditional statements.
 * Parameters:
 *  np      void * The node to start traversing from. Recursively traverses
 *          the children of np.
 * Returns: None
 * Side-effects: None
 */
void traverse_conditional_statement(void *np) {
    Node *n = (Node *) np;
    if (n == NULL) {
        return;
    }
    switch (n->n_type) {
        case IF_THEN:
            fprintf(output, "if ( ");
            traverse_node(n->children.child1);
            fprintf(output, " ) ");
            traverse_node(n->children.child2);
            fprintf(output, "");
            break;
        case IF_THEN_ELSE:
            fprintf(output, "if ( ");
            traverse_node(n->children.child1);
            fprintf(output, " ) ");
            traverse_node(n->children.child2);
            fprintf(output, " else ");
            traverse_node(n->children.child3);
            break;
        default:
            handle_parser_error(PE_UNRECOGNIZED_NODE_TYPE,
                                "traverse_conditional_statement",
                                yylineno);
            break;
    }
}



/*
 * print_data_node
 * Purpose: Helper function for traverse_node.
 *          Traverses nodes with literal data, i.e. strings, chars, and numbers.
 *          Also traverses identifier nodes since they have literal names.
 * Parameters:
 *  np      void * The node to start traversing from.
 * Returns: None
 * Side-effects: None
 */
void print_data_node(void *np) {
    Node *n = (Node *) np;
    switch (n->n_type) {
        case SIMPLE_DECLARATOR:
        case IDENTIFIER_EXPR:
        case IDENTIFIER:
            fprintf(output, "%s", n->data.str);
            break;
        case STRING_CONSTANT:
            /* TODO: replace special characters, e.g. replace newline with \n */
            fprintf(output, "\"%s\"", n->data.str);
            break;
        case NUMBER_CONSTANT:
            fprintf(output, "%d", n->data.num);
            break;
        case CHAR_CONSTANT:
            /* TODO: replace special characters, e.g. replace null with \0 */
            fprintf(output, "'%c'", n->data.ch);
            break;
        default:
            handle_parser_error(PE_INVALID_DATA_TYPE,
                                get_token_name(n->n_type), yylineno);
            break;
        }
}

/* basic helper procs for pretty printing */


void print_pointers(Node *n) {
    if (n == NULL || n->n_type != POINTER) {
        return;
    }
    do {
        fprintf(output, "*");
        n = n->children.child1;
    } while (n != NULL && n->n_type == POINTER);
}

char *get_type_spec(enum data_type type) {
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
            handle_parser_error(PE_UNRECOGNIZED_OP,
                                "get_operator_value", yylineno);
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
            #ifdef DEBUG
            error(0, 0, "line %d: %s: unrecognized node type", line, data);
            #endif
            return;
        case PE_UNRECOGNIZED_OP:
            #ifdef DEBUG
            error(0, 0, "line %d: %s: unrecognized operator", line, data);
            #endif
        default:
            return;
    }
}
