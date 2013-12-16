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

FILE *input, *output;

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
