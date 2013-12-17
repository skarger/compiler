#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../include/ir.h"

#include "../include/cmpl.h"
#include "../include/lexer.h"
#include "../../y.tab.h"
#include "../include/parse-tree.h"
#include "../include/parser.h"
#include "../include/symbol.h"
#include "../include/symbol-collection.h"
#include "../include/symbol-utils.h"
#include "../include/ir.h"

extern SymbolCreationData *scd;
extern IrList *ir_list;

FILE *input, *output;

void test_print_ir(void);

int main(int argc, char *argv[]) {
    extern FILE *yyin;
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
    /* do the work */
    rv = yyparse();
    //test_print_ir();
    fprintf(stdout, "\n");

    /* cleanup */
    if (output != stdout) {
        fclose(output);
    }
    if (input != stdin) {
        fclose(input);
    }

    return rv;
}

void set_int_symbol(Node *n) {
    Symbol *s = create_symbol();
    set_symbol_name(s, n->data.str);
    push_symbol_type(s, SIGNED_INT);
    set_symbol_table_entry(n, s);
}

void test_print_ir(void) {
        IrList *ir_list = create_ir_list();

        Symbol *s;
        YYSTYPE data;
        char str1[] = "a";
        data = (YYSTYPE) create_string(1);
        strcpy( ((struct String *) data)->str, str1 );
        Node *id_expr = create_node(IDENTIFIER_EXPR, data);
        set_int_symbol(id_expr);

        char str2[] = "1";
        data = create_number(str2);
        Node *num_const = create_node(NUMBER_CONSTANT, data);




        Node *bin_expr = create_node(BINARY_EXPR, LOGICAL_OR, id_expr, num_const);
        compute_ir(bin_expr, ir_list);

        Node *assign_expr = create_node(ASSIGNMENT_EXPR, ASSIGN, id_expr, num_const);
        compute_ir(assign_expr, ir_list);

        char str3[] = "f";
        data = (YYSTYPE) create_string(1);
        strcpy( ((struct String *) data)->str, str3 );
        Node *simple_decl = create_node(SIMPLE_DECLARATOR, data);

        set_int_symbol(simple_decl);

        Node *func_decl = create_node(FUNCTION_DECLARATOR, simple_decl, NULL);
        Node *func_def_spec = create_node(FUNCTION_DEF_SPEC, NULL, func_decl);
        compute_ir(func_def_spec, ir_list);

        print_ir_list(stdout, ir_list);
}


extern SymbolCreationData *scd;
extern IrList *ir_list;

/*
 * start_traversal
 * Purpose: Kick off traversal of parse tree. Meant for parser to call.
 * Parameters:
 *  n       Node * The node to start traversing from. Recursively traverses
 *          the children of n.
 * Returns: None
 * Side-effects: Allocates heap memory
 */
void start_traversal(Node *n) {
    if (scd == NULL) {
        util_emalloc((void **) &scd, sizeof(SymbolCreationData));
        initialize_symbol_creation_data(scd);
        scd->outfile = output;
    }
    collect_symbol_data(n, scd);

    start_ir_computation();
    compute_ir(n, ir_list);
    IrNode *irn = ir_list->head;
    print_ir_list(output, ir_list);
}
