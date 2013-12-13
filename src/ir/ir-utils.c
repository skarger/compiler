#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../include/ir.h"
#include "../include/utilities.h"
#include "../../y.tab.h"

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
    irn->instruction = instr;
    switch (instr) {
        case RETURNED_WORD:
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
    irl->tail = NULL;
    irl->cur = NULL;
    return irl;
}

IrNode *append_ir_node(IrNode *irn, IrList *irl) {
    if (irl->head == NULL) {
        irl->head = irn;
        irn->prev = NULL;
    } else {
        irl->tail->next = irn;
        irn->prev = irl->tail;
        irn->next = NULL;
    }
    irn->next = NULL;
    irl->tail = irn;
}

int instruction(IrNode *irn) {
    if (irn == NULL) {
        return NO_IR_INSTRUCTION;
    }
    return irn->instruction;
}

Boolean node_is_lvalue(Node *n) {
    return n->lvalue;
}

void start_ir_computation(void) {
    ir_list = create_ir_list();
}

void compute_ir(Node *n, IrList *irl) {
    if (n == NULL) {
        return;
    }

    switch (n->n_type) {
        case BINARY_EXPR:
            compute_ir(n->children.child1, irl);
            compute_ir(n->children.child2, irl);
            append_ir_node(create_ir_node(STORE_WORD_INDIRECT), irl);
            /* type: assume SIGNED_INT */
            /* lvalue: no */
            /* IR */
            /* location: register */
            break;
        case IDENTIFIER_EXPR:
            n->lvalue = TRUE;
        case NUMBER_CONSTANT:
        case EXPRESSION_STATEMENT:
        case GOTO_STATEMENT:
        case UNARY_EXPR:
        case PREFIX_EXPR:
            break;
        case RETURN_STATEMENT:
            compute_ir(n->children.child1, irl);
            create_ir_node(RETURNED_WORD);
            break;
        default:
            break;
    }
}

void print_ir_node(FILE *out, IrNode *irn) {
    fprintf(out, "(");
    switch(irn->instruction) {
        case RETURNED_WORD:
            fprintf(out, "returnedword");
        default:
            break;
    }
    fprintf(out, ")");
}
