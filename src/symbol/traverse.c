
#include "../include/traverse.h"
#include "../include/symbol-traversal.h"


extern SymbolCreationData *scd;

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
    FILE *output = stdout;
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

}
