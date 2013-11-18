
#ifndef LITERAL_H
#define LITERAL_H

enum data_type {
    NO_DATA_TYPE,
    SIGNED_CHAR,
    UNSIGNED_CHAR,
    SIGNED_SHORT,
    UNSIGNED_SHORT,
    SIGNED_INT,
    UNSIGNED_INT,
    SIGNED_LONG,
    UNSIGNED_LONG,
    OVERFLOW
};

enum Boolean {
    FALSE = 0,
    TRUE = 1
};


struct Character {
    char c;
};

struct String {
    char *str; /* pointer to the beginning of the string */
    char *current; /* pointer to an arbitrary character in str used for construction */
    int length; /* the length of the string */
    enum Boolean valid; /* flag for whether str is valid by the standards of CSCI-E95 C */
};

struct Number {
    unsigned long value;
    enum data_type type;
};

#endif

