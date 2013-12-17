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
static Boolean is_function_call = FALSE;
static Boolean is_function_argument = FALSE;
static IrNode *cur_end_proc_label;

/* file helper functions */
void compute_ir_pass_through(Node *n, IrList *irl);
IrNode *irn_load(int instr, int dest, int src, Symbol *global);
IrNode *irn_store(int instr, int src, int dest);
IrNode *irn_binary_expr(int instr, int dest, int oprnd1, int oprnd2);
IrNode *irn_statement(int instr, int res_idx, IrNode *label);
IrNode *irn_function(int instr, Symbol *function_symbol);
IrNode *irn_param(int instr, int par_reg, int src_reg);
IrNode *irn_label(int instr, int label_idx);



void start_ir_computation(void) {
    ir_list = create_ir_list();
}

/*
 * compute_ir
 * Purpose: given a parse tree with symbol table entries,
 *          compute IR instructions
 *
 * Parameters:
 *  n - Node * - the current node in the parse tree during traversal
 *
 * Returns:
 *  None
 *
 * Side Effects:
 *  Appends IrNodes to the master IrList. Allocates heap memory.
 */
void compute_ir(Node *n, IrList *irl) {
    IrNode *irn1, *irn2, *irn3;
    Node *child1, *child2;
    if (n == NULL) {
        return;
    }

    if (is_statement(n)) {
        reg_idx = 0;
    }

    /* for expressions we will update the parse tree node n:
     * type: assume SIGNED_INT unless node is a symbol with a known type
     * lvalue: yes or no
     * location: register index of its result
     */
    switch (n->n_type) {
        case FUNCTION_DEFINITION:
            /* first child: function def spec */
            /* recurse over it to obtain the function symbol */
            compute_ir(n->children.child1, irl);
            /* now we have appended a BEGIN_PROC node to ir_list */
            /* it has the function symbol */
            cur_end_proc_label = irn_label(LABEL, label_idx++);
            irn2 = irn_function(END_PROC, ir_list->tail->s);
            /* second child: compound statement */
            /* recurse over it to obtain IR nodes for the function body */
            compute_ir(n->children.child2, irl);
            /* finally end the proc */
            append_ir_node(cur_end_proc_label, irl);
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
            irn1 = irn_store(STORE_WORD_INDIRECT,
                    child2->expr->location, child1->expr->location);
            append_ir_node(irn1, irl);
            break;
        case BINARY_EXPR:
            child1 = n->children.child1;
            child2 = n->children.child2;
            compute_ir(child1, irl);
            compute_ir(child2, irl);

            n->expr->lvalue = FALSE;
            n->expr->location = reg_idx++;
            irn1 = irn_binary_expr(LOG_OR, n->expr->location,
                                child2->expr->location, child1->expr->location);
            append_ir_node(irn1, irl);
            break;
        case IDENTIFIER_EXPR:
            n->expr->lvalue = TRUE;
            n->expr->location = reg_idx++;
            if (is_function_call && !is_function_argument) {
                irn1 = irn_function(BEGIN_CALL, n->st_entry);
                append_ir_node(irn1, irl);
            } else {
                irn1 = irn_load(LOAD_ADDRESS,
                        n->expr->location, NO_ARG, n->st_entry);
                append_ir_node(irn1, irl);
                if (is_function_argument) {
                    /* TODO: support more than 1 argument */
                    irn2 = irn_load(LOAD_WORD_INDIRECT,
                                reg_idx, n->expr->location, NULL);
                    irn3 = irn_param(PARAM, 0, reg_idx++);
                    append_ir_node(irn2, irl);
                    append_ir_node(irn3, irl);
                }
            }
            break;
        case NUMBER_CONSTANT:
            n->expr->lvalue = FALSE;
            n->expr->location = reg_idx++;
            irn1 = irn_load(LOAD_CONSTANT,
                    n->expr->location, n->data.num, NULL);
            append_ir_node(irn1, irl);
            if (is_function_argument) {
                irn1 = irn_param(PARAM, 0, n->expr->location);
                append_ir_node(irn1, irl);
            }
            break;
        case RETURN_STATEMENT:
            compute_ir(n->children.child1, irl);
            if (n->children.child1 != NULL) {
                if (n->children.child1->expr->lvalue) {
                    irn1 = irn_load(LOAD_WORD_INDIRECT,
                            reg_idx, n->children.child1->expr->location, NULL);
                    append_ir_node(irn1, irl);
                    irn2 = irn_statement(RETURN_FROM_PROC,
                                reg_idx++, cur_end_proc_label);
                } else {
                    irn2 = irn_statement(RETURN_FROM_PROC,
                        n->children.child1->expr->location, cur_end_proc_label);
                }
                append_ir_node(irn2, irl);
            } else {
                irn1 = irn_statement(RETURN_FROM_PROC,
                                        NO_ARG, cur_end_proc_label);
                append_ir_node(irn1, irl);
            }

            break;
        case FUNCTION_CALL:
            is_function_call = TRUE;
            /* get func symbol to get name and parameters */
            /* will append BEGIN_CALL node */
            compute_ir(n->children.child1, irl);
            Symbol *function_symbol = ir_list->tail->s;
            /* arguments */
            is_function_argument = TRUE;
            compute_ir(n->children.child2, irl);
            is_function_argument = FALSE;

            irn2 = irn_function(CALL, function_symbol);
            irn3 = irn_function(END_CALL, function_symbol);
            append_ir_node(irn2, irl);
            append_ir_node(irn3, irl);
            is_function_call = FALSE;
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

/* IR Node creation functions */
IrNode *construct_ir_node(enum ir_instruction instr) {
    IrNode *irn;
    util_emalloc((void *) &irn, sizeof(IrNode));
    irn->instruction = instr;
    irn->prev = NULL;
    irn->next = NULL;
    irn->branch = NULL;
    irn->IMMVAL = NO_ARG;
    irn->OPRND1 = NO_ARG;
    irn->OPRND2 = NO_ARG;
    irn->RDEST =  NO_ARG;
    irn->RSRC =   NO_ARG;
    irn->LABIDX = NO_ARG;
    irn->s = NULL;
}

IrNode *irn_load(int instr, int dest, int src, Symbol *global) {
    IrNode *irn = construct_ir_node(instr);
    irn->RDEST = dest;
    switch(instr) {
        case LOAD_ADDRESS:
            irn->s = global;
            break;
        case LOAD_WORD_INDIRECT:
            irn->RSRC = src;
            break;
        case LOAD_CONSTANT:
            irn->IMMVAL = src;
            break;
        default:
            break;
    }
    return irn;
}

IrNode *irn_store(int instr, int src, int dest) {
    IrNode *irn = construct_ir_node(instr);
    irn->RDEST = dest;
    switch (instr) {
        case STORE_WORD_INDIRECT:
            irn->RSRC = src;
        default:
            break;
    }
    return irn;
}

IrNode *irn_binary_expr(int instr, int dest, int oprnd1, int oprnd2) {
    IrNode *irn = construct_ir_node(instr);
    irn->RDEST = dest;
    irn->OPRND1 = oprnd1;
    irn->OPRND2 = oprnd2;
    return irn;
}

IrNode *irn_statement(int instr, int res_idx, IrNode *label) {
    IrNode *irn = construct_ir_node(instr);
    irn->RSRC = res_idx;
    irn->branch = label;
    return irn;
}

IrNode *irn_function(int instr, Symbol *function_symbol) {
    IrNode *irn = construct_ir_node(instr);
    irn->s = function_symbol;
    return irn;
}

IrNode *irn_param(int instr, int par_reg, int src_reg) {
    IrNode *irn = construct_ir_node(instr);
    irn->RDEST = par_reg;
    irn->RSRC = src_reg;
    return irn;
}

IrNode *irn_label(int instr, int label_idx) {
    IrNode *irn = construct_ir_node(instr);
    irn->LABIDX = label_idx;
    return irn;
}

/* List of IR Node functions */
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

Boolean is_statement(Node *n) {
    if (n == NULL) {
        return FALSE;
    }
    switch (n->n_type) {
        case FOR_STATEMENT:
        case IF_THEN_ELSE:
        case WHILE_STATEMENT:
        case DO_STATEMENT:
        case IF_THEN:
        case COMPOUND_STATEMENT:
        case EXPRESSION_STATEMENT:
        case RETURN_STATEMENT:
        case GOTO_STATEMENT:
        case BREAK_STATEMENT:
        case CONTINUE_STATEMENT:
        case NULL_STATEMENT:
            return TRUE;
        default:
            return FALSE;
    }
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

/* IR printing functions */
void print_ir_list(FILE *out, IrList *irl) {
    fprintf(out, "\n/*\n");
    fprintf(out, " *** Start IR List ***\n");
    irl->cur = irl->head;
    while (irl->cur != NULL) {
        print_ir_node(out, irl->cur);
        irl->cur = irl->cur->next;
    }
    fprintf(out, " *** End IR List ***\n");
    fprintf(out, " */\n");
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
        case RETURN_FROM_PROC:
            if (irn->RSRC == NO_ARG) {
                fprintf(out, "return, \"LABEL_%d\"", irn->branch->LABIDX);
            } else {
                fprintf(out, "return, \"LABEL_%d\", $r%d",
                        irn->branch->LABIDX, irn->RSRC);
            }
            break;
        case BEGIN_CALL:
            fprintf(out, "begincall, \"%s\"", get_symbol_name(irn->s));
            break;
        case PARAM:
            fprintf(out, "param, %d, $r%d", irn->RDEST, irn->RSRC);
            break;
        case CALL:
            fprintf(out, "call, \"%s\"", get_symbol_name(irn->s));
            break;
        case END_CALL:
            fprintf(out, "endcall, \"%s\"", get_symbol_name(irn->s));
            break;
        case STORE_WORD_INDIRECT:
            fprintf(out, "storewordindirect, $r%d, $r%d",
                            irn->RSRC, irn->RDEST);
            break;
        case LOAD_ADDRESS:
            fprintf(out, "loadaddress, $r%d, %s",
                            irn->RDEST, get_symbol_name(irn->s));
            break;
        case LOAD_WORD_INDIRECT:
            fprintf(out, "loadwordindirect, $r%d, $r%d", irn->RDEST, irn->RSRC);
            break;
        case LOAD_CONSTANT:
            fprintf(out, "loadconstant, $r%d, %d", irn->RDEST, irn->IMMVAL);
            break;
        case LOG_OR:
            fprintf(out, "logicalor, $r%d, $r%d, $r%d",
                            irn->RDEST, irn->OPRND1, irn->OPRND2);
            break;
        case LABEL:
            fprintf(out, "label, \"LABEL_%d\"", irn->LABIDX);
            break;
        default:
            break;
    }
    fprintf(out, ")\n");
}

char *get_ir_name(enum ir_instruction instr) {
    switch (instr) {
    #define CASE_FOR(instr) case instr: return #instr
        CASE_FOR(BEGIN_PROC);
        CASE_FOR(END_PROC);
        CASE_FOR(RETURN_FROM_PROC);
        CASE_FOR(BEGIN_CALL);
        CASE_FOR(PARAM);
        CASE_FOR(CALL);
        CASE_FOR(END_CALL);
        CASE_FOR(STORE_WORD_INDIRECT);
        CASE_FOR(LOAD_ADDRESS);
        CASE_FOR(LOAD_WORD_INDIRECT);
        CASE_FOR(LOAD_CONSTANT);
        CASE_FOR(LOG_OR);
        CASE_FOR(LABEL);
    #undef CASE_FOR
        default: return "";
    }
}
