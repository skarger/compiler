/* error conditions should not happen based on regular expressions but check anyway */ 
int strtochar(char *str, int len) {
    int rv;
    int char_seq_len = len - 2;
    char *buf = malloc(char_seq_len + 1);

    if (str[0] != '\'' || str[len - 1] != '\'') {
        /* error: input not wrapped in '' */
        rv = E_INPUT;
        goto cleanup;
    }

    if (len < 3) {
        /* error: empty or malformed character constant */
        rv = E_INPUT;
        goto cleanup;
    }

    /* trim the surrounding '' characters, leaving room for terminating '\0' */
    strncpy(buf, str+1, char_seq_len);
    buf[char_seq_len] = '\0';

    if (char_seq_len == 1) {
        /* normal case: 'a' */
        rv = buf[0];
        goto cleanup;
    }

    if (char_seq_len > 4) {
        /* error: invalid escape sequence */
        /* longest format is 4 chars e.g. '\377' */
        rv = E_INPUT;
        goto cleanup;
    }

    if (buf[0] == '\\') {
        /* escape code */
        if ( isodigit(buf[1]) ) {
            /* length of octal digit sequence is char_seq_len-1 for the leading \ */
            rv = convert_octal_escape(buf+1, char_seq_len-1);
            goto cleanup;
        }
        /* not an octal escape. that implies it must be one escaped character */
        if (char_seq_len != 2) {
            rv = E_INPUT;
            goto cleanup;
        }
        rv = convert_single_escape(buf[1]);
        goto cleanup;
    }
    /* error: reaching here means sequence longer than 1 char and not a valid escape */
    rv = E_INPUT;

    cleanup:
    free(buf);
    return rv;
}
