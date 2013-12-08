#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../include/ir.h"

FILE *input, *output;

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
    IrNode *irn = create_ir_node(RETURN);
    print_ir_node(stdout, irn);
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
