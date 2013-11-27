#include <stdio.h>
#include <string.h>

#include "../../src/include/traverse.h"

/* define TRAVERSE to for 'make test-symbol-output' to pass */
#define TRAVERSE

extern TraversalData *td;

void symtab_test(SymbolTableContainer *stc);


FILE *output;
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

    /* do the work */
    rv = yyparse();
    /* td should now be set after parsing */
    symtab_test(td->stc);

    /* cleanup */
    if (output != stdout) {
        fclose(output);
    }
    if (input != stdin) {
        fclose(input);
    }

    return rv;
}

void symtab_test(SymbolTableContainer *stc) {
    print_symbol(output, td->stc->current_st->symbols);
    return;
}
