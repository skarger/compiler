#include "../include/ir.h"
#include "../include/scope-fsm.h"
#include "../include/mips.h"

static const char *builtin_print_int = "print_int:\n"
"    li $v0, 1         # v0 <- syscall code for print_int\n"
"    syscall           # print\n"
"    jr $ra            # return to caller\n";

void print_global_variables(FILE *out, SymbolTable *st);

void compute_mips_asm(FILE *output, SymbolTableContainer *stc, IrList *irl) {
    fprintf(output, ".data\n");
    /* write each file scope non-function symbol */
    SymbolTable *st;
    st = stc->symbol_tables[OTHER_NAMES]; /* this is the file level scope ST */
    print_global_variables(output, st);
    /* st = stc->symbol_tables[STATEMENT_LABELS]; */

    fprintf(output, "\n");
    fprintf(output, ".text\n");
    fprintf(output, "\n");
    fprintf(output, "%s", builtin_print_int);
}

void print_global_variables(FILE *out, SymbolTable *st) {
    Symbol *s = st->symbols;
    while (s != NULL) {
        if (symbol_outer_type(s) != FUNCTION) {
            /* TODO: do not assume every symbol is a word */
            fprintf(output, "%s: .word 0\n", get_symbol_name(s));
        }
        s = s->next;
    }
}
