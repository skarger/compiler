
#ifndef LITERAL_H
#define LITERAL_H

#include "parse-tree.h"
#include "utilities.h"

struct Character {
    char c;
};

struct String {
    char *str; /* pointer to the beginning of the string */
    char *current; /* pointer to an arbitrary character in str used for construction */
    int length; /* the length of the string */
    boolean valid; /* flag for whether str is valid by the standards of CSCI-E95 C */
};

struct Number {
    unsigned long value;
    enum data_type type;
};

#endif

