/*
 * util_emalloc
 * Purpose:
 *      Allocate heap memory and store it in the passed in pointer.
 * Parameters:
 *      ptr - pointer to pointer that should be set to malloc's return value.
 *      n - the number of bytes to allocate.
 * Returns:
 *      None
 * Side effects:
 *      Sets the value of ptr.
 *      Terminates program if malloc errors.
 */
#include <error.h>
#include <stdlib.h>
#include "../include/utilities.h"

void util_emalloc(void **ptr, size_t n) {
    if ( (*ptr = malloc(n)) == NULL ) {
        util_handle_error(UE_MALLOC, "util_emalloc");
    }
}


/*
 * util_handle_error
 * Purpose:
 *      Handle an error caught in the calling method.
 * Parameters:
 *      e - the error value. if non-zero the program will exit with status e
 *      data - string that will be inserted into the message printed to stderr
 * Returns:
 *      None
 * Side effects:
 *      May terminates program depending on e
 */
void util_handle_error(enum util_error e, char *data) {
    switch (e) {
        case UE_SUCCESS:
            return;
        case UE_MALLOC:
            error(e, 0, "%s: out of memory", data);
            return;
        default:
            return;
    }
}
