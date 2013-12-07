#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../include/ir.h"
#include "../include/utilities.h"

FILE *output;

int reg_idx = 0;

int m(int argc, char *argv[]) {
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


    /* do the work */
    //rv = yyparse();


    /* cleanup */
    if (output != stdout) {
        fclose(output);
    }
    if (input != stdin) {
        fclose(input);
    }

    return rv;
}

char *current_reg(void) {
    char *buf;
    util_emalloc((void **) &buf, MAX_REG_LEN);
    snprintf(buf, MAX_REG_LEN, "$r%d", reg_idx);
    return buf;
}

char *next_reg() {
    char *buf;
    util_emalloc((void **) &buf, MAX_REG_LEN);
    snprintf(buf, MAX_REG_LEN, "$r%d", ++reg_idx);
    return buf;
}
