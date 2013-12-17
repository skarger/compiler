
#include "../include/cmpl.h"
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
    #ifdef COLLECT_SYMBOLS
    if (scd == NULL) {
        util_emalloc((void **) &scd, sizeof(SymbolCreationData));
        initialize_symbol_creation_data(scd);
        scd->outfile = output;
    }
    collect_symbol_data(n, scd);
    #endif

    #ifdef PRETTY_PRINT
    pretty_print(n);
    #endif

    #ifdef COMPUTE_IR
    start_ir_computation();
    compute_ir(n, ir_list);
    IrNode *irn = ir_list->head;
        #ifdef PRINT_IR
        print_ir_list(output, ir_list);
        #endif
    #endif

    #ifdef MIPS_ASM
    compute_mips_asm(output, scd->stc, ir_list);
    #endif
}
