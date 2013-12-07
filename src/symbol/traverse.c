
#include "../include/traverse.h"
#include "../include/symbol-traversal.h"


/* external variable for traversal data. this is the defining declaration. */
TraversalData *td = NULL;

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
    if (td == NULL) {
        util_emalloc((void **) &td, sizeof(TraversalData));
        initialize_traversal_data(td);
        td->outfile = output;
    }
    traverse_node(n, td);
}
