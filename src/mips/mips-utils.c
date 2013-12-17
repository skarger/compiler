#include "../include/ir.h"
#include "../include/scope-fsm.h"
#include "../include/mips.h"

static const char *syscall_print_int = "syscall_print_int:\n"
"    li $v0, 1         # v0 <- syscall code for print_int\n"
"    syscall           # print\n"
"    jr $ra            # return to caller\n";

static const char *main_intro ="    addiu   $sp, $sp, -48"
"  # push space for our stack frame onto the stack\n"
"    sw $fp, 44($sp)        # save the old $fp\n"
"    addiu $fp, $sp, 44     # $fp -> stack frame\n"
"    sw $ra, -4($fp)        # save the return address\n";

static const char *main_outro = "    lw  $ra, -4($fp)       # restore $ra\n"
"    lw  $fp, ($fp)         # restore old $fp\n"
"    addiu   $sp, $sp, 48   # pop off our stack frame\n"
"    jr $ra\n";


void print_global_variables(FILE *out, SymbolTable *st);
void print_functions(FILE *out, SymbolTableContainer *stc, IrList *irl);
void ir_to_mips(FILE *out, IrNode *irn);

void compute_mips_asm(FILE *output, SymbolTableContainer *stc, IrList *irl) {
    fprintf(output, "    .data\n");
    /* write each file scope non-function symbol */
    SymbolTable *st;
    st = stc->symbol_tables[OTHER_NAMES]; /* this is the file level scope ST */
    print_global_variables(output, st);
    /* st = stc->symbol_tables[STATEMENT_LABELS]; */

    fprintf(output, "\n");
    fprintf(output, "    .text\n");
    /* print each function */
    print_functions(output, stc, irl);

    fprintf(output, "\n");
    fprintf(output, "%s", syscall_print_int);
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
            #ifdef PROCEDURE_CALLS_SUPPORTED
                /* procedure entry steps */
            #else
            fprintf(out, "%s", main_intro);
            #endif
            break;
        case RETURN_FROM_PROC:
            if (irn->n1 != NR) {
                fprintf(out, "    lw    $v0, ($r%d)\n", irn->n1);
            }
            fprintf(out, "    j     LABEL_%d\n", irn->branch->n1);
            break;
        case END_PROC:
            #ifdef PROCEDURE_CALLS_SUPPORTED
                /* procedure completion steps */
            #else
            fprintf(out, "%s", main_outro);
            #endif
            break;
        case LOAD_ADDRESS:
            fprintf(out, "    la    $t%d, %s\n", irn->n1, get_symbol_name(irn->s));
            break;
        case LOAD_WORD_INDIRECT:
            fprintf(out, "    lw    $t%d, ($t%d)\n", irn->n1, irn->n2);
            break;
        case LOAD_CONSTANT:
            fprintf(out, "    li    $t%d, %d\n", irn->n1, irn->n2);
            break;
        case STORE_WORD_INDIRECT:
            fprintf(out, "    sw    $t%d, ($t%d)\n", irn->n1, irn->n2);
            break;
        case LABEL:
            fprintf(out, "LABEL_%d:\n", irn->n1);
            break;
        case BEGIN_CALL:
            fprintf(out, "    addiu $sp, $sp, -4 # push space for argument\n");
            break;
        case PARAM:
            fprintf(out, "    or    $a%d, $t%d, $0\n", irn->n1, irn->n2);
            break;
        case CALL:
            fprintf(out, "    jal %s\n", get_symbol_name(irn->s));
            break;
        case END_CALL:
            fprintf(out, "    addiu $sp, $sp, 4 # pop off space for argument\n");
            break;
        default:
            fprintf(out, "unknown instruction\n");
            break;
    }
}
