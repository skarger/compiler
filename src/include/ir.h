#ifndef IR_H
#define IR_H

#include "parse-tree.h"

#define MAX_REG_LEN 24
#define NR -1 /* no register */

#define CHAR_BYTES 1
#define SHORT_BYTES 2
#define INT_BYTES 4
#define LONG_BYTES 4


struct IrNode {
    struct IrNode *prev;
    struct IrNode *next;
    enum ir_instruction instruction;
    int res_idx;
    int arg1_idx;
    int arg2_idx;
    Symbol *s;
    int num; /* constant maybe delete */
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
    LOAD_WORD,
    LOAD_CONSTANT,
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


void start_ir_computation(void);
void compute_ir(Node *n, IrList *irl);

char *current_reg(void);
char *next_reg(void);
IrNode *create_ir_node(int instr, int res_idx, int arg1_idx, int arg2_idx, char *label, int num);
IrList *create_ir_list(void);
IrNode *append_ir_node(IrNode *irn, IrList *irl);
int instruction(IrNode *irn);
Boolean node_is_lvalue(Node *n);


void print_ir_list(FILE *out, IrList *irl);
void print_ir_node(FILE *out, IrNode *irn);


#endif
