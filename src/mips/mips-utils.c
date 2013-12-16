#include "../include/ir.h"
#include "../include/scope-fsm.h"
#include "../include/mips.h"

static const char *builtin_print_int = "print_int:\n"
"    li $v0, 1         # v0 <- syscall code for print_int\n"
"    syscall           # print\n"
"    jr $ra            # return to caller\n";

void print_global_variables(FILE *out, SymbolTable *st);
void print_functions(FILE *out, SymbolTableContainer *stc, IrList *irl);
void ir_to_mips(FILE *out, IrNode *irn);

void compute_mips_asm(FILE *output, SymbolTableContainer *stc, IrList *irl) {
    fprintf(output, ".data\n");
    /* write each file scope non-function symbol */
    SymbolTable *st;
    st = stc->symbol_tables[OTHER_NAMES]; /* this is the file level scope ST */
    print_global_variables(output, st);
    /* st = stc->symbol_tables[STATEMENT_LABELS]; */

    fprintf(output, "\n");
    fprintf(output, ".text\n");
    /* print each function */
    print_functions(output, stc, irl);

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

void print_functions(FILE *out, SymbolTableContainer *stc, IrList *irl) {
    IrNode *cur = irl->head;
    while (cur != NULL) {
        ir_to_mips(out, cur);
        cur = cur->next;
    }
}

void ir_to_mips(FILE *out, IrNode *irn) {
    switch(irn->instruction) {
        case BEGIN_PROC:
            fprintf(out, "%s:\n", get_symbol_name(irn->s));
            fprintf(out, "    sw    $ra, -4($fp)\n");
            break;
        case END_PROC:
            fprintf(out, "    lw    $ra, -4($fp)    # restore $ra\n");
            fprintf(out, "    jr    $ra\n");
            break;
        default:
            fprintf(out, "unknown instruction\n");
            break;
    }
}
