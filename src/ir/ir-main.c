#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../include/ir.h"

#include "../include/traverse.h"
#include "../include/lexer.h"
#include "../../y.tab.h"
#include "../include/parse-tree.h"
#include "../include/parser.h"
#include "../include/symbol-utils.h"




FILE *input, *output;

void test_print_ir(void);

int main(int argc, char *argv[]) {

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


    /* do the work */
    //rv = yyparse();
    test_print_ir();
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

void test_print_ir(void) {
        Symbol *s;
        YYSTYPE data;
        char str1[] = "a";
        data = (YYSTYPE) create_string(1);
        strcpy( ((struct String *) data)->str, str1 );
        Node *id_expr = create_node(IDENTIFIER_EXPR, data);
        s = create_symbol();
        set_symbol_name(s, id_expr->data.str);
        push_symbol_type(s, SIGNED_INT);
        set_symbol_table_entry(id_expr, s);

        char str2[] = "1";
        data = create_number(str2);
        Node *num_const = create_node(NUMBER_CONSTANT, data);

        /* Node *bin_expr = create_node(ASSIGNMENT_EXPR, ASSIGN, id_expr, num_const); */
        Node *bin_expr = create_node(BINARY_EXPR, LOGICAL_OR, id_expr, num_const);

        IrList *ir_list = create_ir_list();

        compute_ir(bin_expr, ir_list);
        print_ir_list(stdout, ir_list);
}
