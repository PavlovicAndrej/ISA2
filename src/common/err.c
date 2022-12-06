/**
 * @Author Andrej Pavlovič
 * @Email <xpavlo14@vutbr.cz>
 * @Project DNS Tunneling
 * @Program Error handling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void err_handle(char const *const msg, int const ex) {
    if (errno) {
        perror(msg ? msg : "");
    } else if (msg && strcmp(msg, "")) {
        fprintf(stderr, "%s\n", msg);
    } else {
        fprintf(stderr, "unknown err\n");
    }
    if (ex) {
        exit(EXIT_FAILURE);
    } else {
        errno = 0;
    }
}