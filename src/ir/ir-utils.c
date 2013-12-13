#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../include/ir.h"
#include "../include/utilities.h"
#include "../../y.tab.h"

FILE *output;

static int reg_idx = 0;
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

IrNode *create_ir_node(int instr, int res_idx, int arg1_idx, int arg2_idx, char *label, int num) {
    IrNode *irn;
    util_emalloc((void *) &irn, sizeof(IrNode));
    irn->instruction = instr;
    irn->res_idx;
    irn->arg1_idx;
    irn->arg2_idx;
    irn->label = label;
    irn->num = num;
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
            append_ir_node(create_ir_node(STORE_WORD_INDIRECT, ++reg_idx, 0, 0, "", 0), irl);
            /* type: assume SIGNED_INT */
            /* lvalue: no */
            /* IR */
            /* location: register */
            break;
        case IDENTIFIER_EXPR:
            n->lvalue = TRUE;
            /* get name from symbol */
            append_ir_node(create_ir_node(LOAD_WORD_INDIRECT, ++reg_idx, 0, 0, n->data.str, 0), irl);
            break;
        case NUMBER_CONSTANT:
            n->lvalue = FALSE;
            append_ir_node(create_ir_node(LOAD_CONSTANT, ++reg_idx, 0, 0, "", n->data.num), irl);
            break;
        case EXPRESSION_STATEMENT:
        case GOTO_STATEMENT:
        case UNARY_EXPR:
        case PREFIX_EXPR:
            break;
        case RETURN_STATEMENT:
            compute_ir(n->children.child1, irl);
            create_ir_node(RETURNED_WORD, 0, 0, 0, "", 0);
            break;
        default:
            break;
    }
}

void print_ir_list(FILE *out, IrList *irl) {
    irl->cur = irl->head;
    while (irl->cur != NULL) {
        print_ir_node(stdout, irl->cur);
        irl->cur = irl->cur->next;
    }
}

void print_ir_node(FILE *out, IrNode *irn) {
    fprintf(out, "(");
    switch(irn->instruction) {
        case RETURNED_WORD:
            fprintf(out, "returnedword");
            break;
        case STORE_WORD_INDIRECT:
            fprintf(out, "storewordindirect, $r%d, $r%d", irn->arg1_idx, irn->res_idx);
            break;
        case LOAD_WORD_INDIRECT:
            fprintf(out, "loadwordindirect, $r%d, %s", irn->res_idx, irn->label);
            break;
        case LOAD_CONSTANT:
            fprintf(out, "loadconstant, $r%d, %d", irn->res_idx, irn->num);
            break;
        default:
            break;
    }
    fprintf(out, ")\n");
}
