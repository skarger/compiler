#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../include/ir.h"
#include "../include/utilities.h"
#include "../../y.tab.h"
#include "../include/parse-tree.h"

FILE *output;

/* external variable. this is the defining declaration. */
IrList *ir_list = NULL;

static int reg_idx = 0;
static int label_idx = 0;
static Boolean is_function_def_spec = FALSE;

/* helper functions */
void compute_ir_pass_through(Node *n, IrList *irl);

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

IrNode *irn_binary_expr(int instr, int to_idx, int oprnd1, int oprnd2) {
    return create_ir_node(instr, to_idx, oprnd1, oprnd2, NULL);
}

IrNode *irn_statement(int instr, int res_idx) {
    return create_ir_node(instr, res_idx, NR, NR, NULL);
}

IrNode *irn_function(int instr, Symbol *s) {
    return create_ir_node(instr, NR, NR, NR, s);
}

IrNode *irn_label(int instr, int label_idx) {
    return create_ir_node(instr, label_idx, NR, NR, NULL);
}

IrNode *create_ir_node(int instr, int n1, int n2, int n3, Symbol *s) {
    IrNode *irn;
    util_emalloc((void *) &irn, sizeof(IrNode));
    irn->instruction = instr;
    irn->n1 = n1;
    irn->n2 = n2;
    irn->n3 = n3;
    irn->s = s;
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
    }
    irn->next = NULL;
    irl->tail = irn;
    return irl->tail;
}

IrNode *prepend_ir_node(IrNode *irn, IrList *irl) {
    if (irl->head == NULL) {
        irl->head = irn;
        irn->next = NULL;
    } else {
        irl->head->prev = irn;
        irn->next = irl->head;
    }
    irn->prev = NULL;
    irl->head = irn;
    return irl->head;
}

int instruction(IrNode *irn) {
    if (irn == NULL) {
        return NO_IR_INSTRUCTION;
    }
    return irn->instruction;
}

Boolean node_is_lvalue(Node *n) {
    return n->expr->lvalue;
}

void start_ir_computation(void) {
    ir_list = create_ir_list();
}

void compute_ir(Node *n, IrList *irl) {
    IrNode *irn1, *irn2, *irn3;
    Node *child1, *child2;
    if (n == NULL) {
        return;
    }

    /* append IrNodes to the IrList  */

    /* for expressions compute_ir will update the node: */
    /* type: assume SIGNED_INT unless node is a symbol with a known type */
    /* lvalue: yes or no */
    /* location: register index of its result */

    switch (n->n_type) {
        case FUNCTION_DEFINITION:
            /* first child: function def spec */
            /* recurse over it to obtain the function symbol */
            compute_ir(n->children.child1, irl);
            /* now we have appended a BEGIN_PROC node to ir_list */
            /* it has the function symbol */
            irn1 = irn_label(LABEL, label_idx++);
            irn2 = irn_function(END_PROC, ir_list->tail->s);
            /* second child: compound statement */
            /* recurse over it to obtain IR nodes for the function body */
            compute_ir(n->children.child2, irl);
            /* finally end the proc */
            append_ir_node(irn1, irl);
            append_ir_node(irn2, irl);
            break;
        case FUNCTION_DEF_SPEC:
            is_function_def_spec = TRUE;
            /* only need to recurse over the declarator */
            /* to go get the function symbol */
            compute_ir(n->children.child2, irl);
            is_function_def_spec = FALSE;
            break;
        case POINTER_DECLARATOR:
            if (is_function_def_spec) {
                compute_ir(n->children.child2, irl);
            }
        case FUNCTION_DECLARATOR:
            if (is_function_def_spec) {
                compute_ir(n->children.child1, irl);
            }
            break;
        case SIMPLE_DECLARATOR:
            if (is_function_def_spec) {
                irn1 = irn_function(BEGIN_PROC, n->st_entry);
                append_ir_node(irn1, irl);
            }
            break;
        case ASSIGNMENT_EXPR:
            child1 = n->children.child1;
            child2 = n->children.child2;
            compute_ir(child1, irl);
            compute_ir(child2, irl);
            irn1 = irn_store_number_constant(STORE_WORD_INDIRECT,
                            child2->expr->location, child1->expr->location);
            append_ir_node(irn1, irl);
            break;
        case BINARY_EXPR:
            child1 = n->children.child1;
            child2 = n->children.child2;
            compute_ir(child1, irl);
            compute_ir(child2, irl);

            n->expr->lvalue = FALSE;
            n->expr->location = ++reg_idx;
            irn1 = irn_binary_expr(LOG_OR, n->expr->location,
                                child2->expr->location, child1->expr->location);
            append_ir_node(irn1, irl);
            break;
        case IDENTIFIER_EXPR:
            n->expr->lvalue = TRUE;
            n->expr->location = ++reg_idx;
            /* get name from symbol */
            irn1 = irn_load_from_global(LOAD_ADDRESS,
                        n->expr->location, n->st_entry);
            append_ir_node(irn1, irl);
            break;
        case NUMBER_CONSTANT:
            n->expr->lvalue = FALSE;
            n->expr->location = ++reg_idx;
            irn1 = irn_load_number_constant(LOAD_CONSTANT,
                        n->expr->location, n->data.num);
            append_ir_node(irn1, irl);
            break;
        case RETURN_STATEMENT:
            compute_ir(n->children.child1, irl);
            if (n->children.child1 != NULL) {
                irn1 = irn_statement(RETURN, n->children.child1->expr->location);
            } else {
                irn1 = irn_statement(RETURN, NR);
            }
            append_ir_node(irn1, irl);
            break;
        default:
            compute_ir_pass_through(n, irl);
    }
}

/* nodes to simply pass through with respect to IR generation */
void compute_ir_pass_through(Node *n, IrList *irl) {
    if (n == NULL) {
        return;
    }
    switch (n->n_type) {
        case FOR_STATEMENT:
            compute_ir(n->children.child1, irl);
            compute_ir(n->children.child2, irl);
            compute_ir(n->children.child3, irl);
            compute_ir(n->children.child4, irl);
            break;
        case CONDITIONAL_EXPR:
        case IF_THEN_ELSE:
            compute_ir(n->children.child1, irl);
            compute_ir(n->children.child2, irl);
            compute_ir(n->children.child3, irl);
            break;
        case TRANSLATION_UNIT:
        case DECL:
        case PTR_ABS_DECL:
        case DECL_OR_STMT_LIST:
        case WHILE_STATEMENT:
        case DO_STATEMENT:
        case IF_THEN:
        case CAST_EXPR:
        case TYPE_NAME:
        case SUBSCRIPT_EXPR:
        case FUNCTION_CALL:
        case POSTFIX_EXPR:
            compute_ir(n->children.child1, irl);
            compute_ir(n->children.child2, irl);
            break;
        case COMPOUND_STATEMENT:
        case EXPRESSION_STATEMENT:
        case RETURN_STATEMENT:
        case GOTO_STATEMENT:
        case UNARY_EXPR:
        case PREFIX_EXPR:
            compute_ir(n->children.child1, irl);
            break;
        case BREAK_STATEMENT:
        case CONTINUE_STATEMENT:
        case NULL_STATEMENT:
        case CHAR_CONSTANT:
        case STRING_CONSTANT:
            break;
        default:
            /* error? */
            break;
    }
}

void print_ir_list(FILE *out, IrList *irl) {
    irl->cur = irl->head;
    if (irl->cur == NULL) {
        printf("irl cur NULL\n");
    }
    while (irl->cur != NULL) {
        print_ir_node(stdout, irl->cur);
        irl->cur = irl->cur->next;
    }
}

void print_ir_node(FILE *out, IrNode *irn) {
    fprintf(out, "(");
    switch(irn->instruction) {
        case BEGIN_PROC:
            fprintf(out, "beginproc, \"%s\"", get_symbol_name(irn->s));
            break;
        case END_PROC:
            fprintf(out, "endproc, \"%s\"", get_symbol_name(irn->s));
            break;
        case RETURN:
            if (irn->n1 == NR) {
                fprintf(out, "return");
            } else {
                fprintf(out, "return, $r%d", irn->n1);
            }
            break;
        case STORE_WORD_INDIRECT:
            fprintf(out, "storewordindirect, $r%d, $r%d", irn->n1, irn->n2);
            break;
        case LOAD_ADDRESS:
            fprintf(out, "loadaddress, $r%d, %s", irn->n1, get_symbol_name(irn->s));
            break;
        case LOAD_WORD_INDIRECT:
            fprintf(out, "loadwordindirect, $r%d, %s", irn->n1, irn->n2);
            break;
        case LOAD_CONSTANT:
            fprintf(out, "loadconstant, $r%d, %d", irn->n1, irn->n2);
            break;
        case LOG_OR:
            fprintf(out, "logicalor, $r%d, $r%d, $r%d", irn->n1, irn->n2, irn->n3);
            break;
        case LABEL:
            fprintf(out, "label, \"LABEL_%d\"", irn->n1);
            break;
        default:
            break;
    }
    fprintf(out, ")\n");
}
