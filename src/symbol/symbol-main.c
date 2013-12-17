#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "../include/parse-tree.h"
#include "../include/symbol.h"
#include "../include/symbol-collection.h"

extern SymbolCreationData *scd;

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

    /* cleanup */
    if (output != stdout) {
        fclose(output);
    }
    if (input != stdin) {
        fclose(input);
    }

    return rv;
}

void start_traversal(Node *n) {
    if (scd == NULL) {
        util_emalloc((void **) &scd, sizeof(SymbolCreationData));
        initialize_symbol_creation_data(scd);
        scd->outfile = output;
    }
    collect_symbol_data(n, scd);

    pretty_print(n);
}
