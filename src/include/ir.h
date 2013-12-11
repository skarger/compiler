#ifndef IR_H
#define IR_H

#include "parse-tree.h"

#define MAX_REG_LEN 24

struct IrNode {
    struct IrNode *prev;
    struct IrNode *next;
    int instruction;
    int arg1_idx;
    int arg2_idx;
    int res_idx;
};
typedef struct IrNode IrNode;

struct IrList {
    IrNode *head;
    IrNode *tail;
    IrNode *cur;
};
typedef struct IrList IrList;

enum ir_instruction {
    NO_IR_INSTRUCTION,
    LOAD_ADDR,
    LOAD_BYTE_INDIRECT,
    LOAD_HALF_WORD_INDIRECT,
    LOAD_WORD_INDIRECT,
    LOAD_CONST,
    LOAD_INT,
    PARAMETER,
    BEGIN_PROC,
    END_PROC,
    CALL,
    SYSCALL,
    RETURNED_WORD,
    STORE_WORD,
    STORE_WORD_INDIRECT,
    ADD_CONST,
    JUMP,
    LABEL,
    JUMP_EQZ,
    JUMP_NEZ,
    JUMP_LEZ,
    JUMP_GEZ,
    ADD,
    SUB,
    MULT,
    ADDU,
    SUBU
};


void compute_ir(Node *n, IrList *irl);

char *current_reg(void);
char *next_reg(void);
IrNode *create_ir_node(int instr);
IrList *create_ir_list(void);
IrNode *append_ir_node(IrNode *irn, IrList *irl);
int instruction(IrNode *irn);



void print_ir_node(FILE *out, IrNode *irn);

#endif
