/*
 * Definitions, enumerations, structures and function prototypes
 * used for utilities used in multiple compiler components.
 */
#ifndef UTILITIES_H
#define UTILITIES_H

#include <sys/types.h>

/* conveniently our 4 byte long implies 10 digits max as well 4,294,967,295 */
#define MAX_MESSAGE_DIGITS 10

enum Boolean {
    FALSE = 0,
    TRUE = 1
};
typedef enum Boolean boolean;

enum util_error {
    UE_SUCCESS = 0,
    UE_MALLOC = -1,
};

void util_handle_error(enum util_error e, char *data);
void util_emalloc(void **ptr, size_t n);
char *util_get_type_spec(int type);
char *util_compose_numeric_message(char *fmt, long num);

#endif
