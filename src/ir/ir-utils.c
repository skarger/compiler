#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../include/ir.h"
#include "../include/utilities.h"

FILE *output;

int reg_idx = 0;
IrList *ir_list = NULL;


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

IrNode *create_ir_node(int instr) {
    IrNode *irn;
    util_emalloc((void *) &irn, sizeof(IrNode));
    irn->instruction = RETURN;
    switch (instr) {
        case RETURN:
            break;
        default:
            break;
    }

    return irn;
}

IrList *create_ir_list(void) {
    IrList *irl;
    util_emalloc((void **) &irl, sizeof(IrList));
    irl->head = NULL;
    irl->cur = NULL;
    return irl;
}



void compute_ir(Node *n, IrList *irl) {
    if (n == NULL) {
        return;
    }

    if (irl == NULL) {
        irl = create_ir_list();
    }

    switch (n->n_type) {
        case EXPRESSION_STATEMENT:
        case GOTO_STATEMENT:
        case UNARY_EXPR:
        case PREFIX_EXPR:
            break;
        case RETURN_STATEMENT:
            compute_ir(n->children.child1, irl);
            create_ir_node(RETURN);
            break;
        default:
            break;
    }
}

void print_ir_node(FILE *out, IrNode *irn) {
    fprintf(out, "(");
    switch(irn->instruction) {
        case RETURN:
            fprintf(out, "return");
        default:
            break;
    }
    fprintf(out, ")");
}
