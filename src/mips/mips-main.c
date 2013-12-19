#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../include/cmpl.h"
#include "../include/lexer.h"
#include "../../y.tab.h"
#include "../include/parse-tree.h"
#include "../include/parser.h"
#include "../include/symbol-utils.h"
#include "../include/ir.h"
#include "../include/symbol-collection.h"
#include "../include/symbol.h"
#include "../include/mips.h"


extern SymbolCreationData *scd;
extern IrList *ir_list;

FILE *input, *output;

int yyparse(void);

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

    compute_mips_asm(output, scd->stc, ir_list);
}
