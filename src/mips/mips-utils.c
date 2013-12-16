#include "../include/ir.h"
#include "../include/mips.h"

static const char *builtin_print_int = "print_int:\n"
"    li $v0, 1         # v0 <- syscall code for print_int\n"
"    syscall           # print\n"
"    jr $ra            # return to caller\n";

void compute_mips_asm(FILE *output, IrList *irl) {
    fprintf(output, ".data\n");
    /* write each file scope non-function symbol */
    fprintf(output, ".text\n");
    fprintf(output, "\n");
    fprintf(output, "%s", builtin_print_int);
}
