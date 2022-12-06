/**
 * @Author Andrej Pavlovič
 * @Email <xpavlo14@vutbr.cz>
 * @Project DNS Tunneling
 * @Program Functions shared by multiple applications in project, used for program's arguments handling.
 */
 
#include <ctype.h>

#include "err.h"

void check_host_lex(const char *host) {
    unsigned char cnt_d, cnt_l; // character counters for domain and label
    cnt_d = cnt_l = 0;

    if (*host == '-')
        err_handle("Invalid syntax of base domain (label starts with forbidden hyphen)", EXIT);
    for (; *host; host++, cnt_d++, cnt_l++) {
        if (cnt_l > 63)
            err_handle("Invalid syntax of base domain (label name too long >63)", EXIT);
        if (!isalnum(*host) && *host != '-' && *host != '.')
            err_handle("Invalid syntax of base domain (forbidden character(s))", EXIT);
        if (*host == '.') {
            if (*(host + 1) == '.')
                err_handle("Invalid syntax of base domain (empty domain)", EXIT);
            if (*(host + 1) == '-' || *(host - 1) == '-')
                err_handle("Invalid syntax of base domain (label starts/ends with hyphen)", EXIT);
            cnt_l = 0;
        }
    }
    if (*(host - 1) == '-')
        err_handle("Invalid syntax of base domain (label ends with forbidden hyphen)",EXIT);
    if (cnt_d > 251)
        err_handle("Invalid syntax of base domain (domain too long >251)", EXIT);
}