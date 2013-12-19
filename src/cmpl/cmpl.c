
#include "../include/cmpl.h"
#include "../../src/include/parser.h"
#include "../include/symbol-collection.h"
#include "../include/symbol.h"
#include "../include/ir.h"
#include "../include/mips.h"

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

    pretty_print(n);

    start_ir_computation();
    compute_ir(n, ir_list);
    IrNode *irn = ir_list->head;
    print_ir_list(output, ir_list);

    compute_mips_asm(output, scd->stc, ir_list);
}
