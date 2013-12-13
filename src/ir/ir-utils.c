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

IrNode *irn_load_from_register(int instr, int to_idx, int from_idx) {
    return create_ir_node(instr, to_idx, from_idx, NR, NULL);
}

IrNode *irn_store_from_register(int instr, int from_idx, int to_idx) {
    return create_ir_node(instr, from_idx, to_idx, NR, NULL);
}

IrNode *irn_load_from_global(int instr, int to_idx, Symbol *s) {
    return create_ir_node(instr, to_idx, NR, NR, s);
}

IrNode *irn_load_number_constant(int instr, int to_idx, int val) {
    return create_ir_node(instr, to_idx, val, NR, NULL);
}

IrNode *irn_store_number_constant(int instr, int val, int to_idx) {
    return create_ir_node(instr, val, to_idx, NR, NULL);
}

IrNode *create_ir_node(int instr, int n1, int n2, int n3, Symbol *s) {
    IrNode *irn;
    util_emalloc((void *) &irn, sizeof(IrNode));
    irn->instruction = instr;
    irn->n1 = n1;
    irn->n2 = n2;
    irn->n3 = n3;
    irn->s = s;
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

    return irl->tail;
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
            append_ir_node(irn_store_number_constant(STORE_WORD_INDIRECT, 0, ++reg_idx), irl);
            /* type: assume SIGNED_INT */
            /* lvalue: no */
            /* IR */
            /* location: register */
            break;
        case IDENTIFIER_EXPR:
            n->lvalue = TRUE;
            /* get name from symbol */
            append_ir_node(irn_load_from_global(LOAD_WORD_INDIRECT, ++reg_idx, n->st_entry), irl);
            break;
        case NUMBER_CONSTANT:
            n->lvalue = FALSE;
            append_ir_node(irn_load_number_constant(LOAD_CONSTANT, ++reg_idx, n->data.num), irl);
            break;
        case EXPRESSION_STATEMENT:
        case GOTO_STATEMENT:
        case UNARY_EXPR:
        case PREFIX_EXPR:
            break;
        case RETURN_STATEMENT:
            compute_ir(n->children.child1, irl);
            create_ir_node(RETURNED_WORD, 0, 0, 0, NULL);
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
            fprintf(out, "storewordindirect, $r%d, $r%d", irn->n1, irn->n2);
            break;
        case LOAD_WORD_INDIRECT:
            fprintf(out, "loadwordindirect, $r%d, %s", irn->n1, get_symbol_name(irn->s));
            break;
        case LOAD_CONSTANT:
            fprintf(out, "loadconstant, $r%d, %d", irn->n1, irn->n2);
            break;
        default:
            break;
    }
    fprintf(out, ")\n");
}
