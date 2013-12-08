#ifndef IR_H
#define IR_H

#include "parse-tree.h"

#define MAX_REG_LEN 24

struct IrNode {
    struct IrNode *prev;
    struct IrNode *next;
    int instruction;
    int reg_idx1;
    int reg_idx2;
    int reg_idx3;
};
typedef struct IrNode IrNode;

struct IrList {
    IrNode *head;
    IrNode *cur;
};
typedef struct IrList IrList;

enum ir_instruction {
    LOAD_ADDR,
    LOAD_CONST,
    LOAD_INT,
    LOAD_WORD_INDIRECT,
    PARAMETER,
    BEGIN_PROC,
    END_PROC,
    CALL,
    SYSCALL,
    RETURN,
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


char *current_reg(void);
char *next_reg(void);
IrNode *create_ir_node(int instr);
IrList *create_ir_list(void);
void compute_ir(Node *n, IrList *irl);

void print_ir_node(FILE *out, IrNode *irn);

#endif
