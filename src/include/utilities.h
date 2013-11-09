/*
 * Definitions, enumerations, structures and function prototypes
 * used for utilities used in multiple compiler components.
 */
#ifndef UTILITIES_H
#define UTILITIES_H

#include <sys/types.h>

enum util_error {
    UE_SUCCESS = 0,
    UE_MALLOC = -1,
};

void util_handle_error(enum util_error e, char *data);
void util_emalloc(void **ptr, size_t n);

#endif
