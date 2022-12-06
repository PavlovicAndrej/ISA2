/**
 * @Author Andrej Pavlovič
 * @Email <xpavlo14@vutbr.cz>
 * @Project DNS Tunneling
 * @Program Error handling.
 * @details header file
 */

// GUARD
#ifndef ERR_H
#define ERR_H

#define EXIT 1
#define WARNING 0

/** Macro prints detailed information about position in code, at which it is called. */
#define ERR_INFO() fprintf(stderr, "err:%s:%d: in %s()\n", __FILE__, __LINE__, __func__)

/**
 * Prints error message on standard error and optionally exits program with EXIT_FAILURE status.
 *
 * If global variable 'errno' is non-zero, prints error message represented by it's value.
 * If parameter 'msg' is not NULL or empty string, prints it.
 * If all of above printings are omitted, prints "unknown err".
 *
 * @param msg Error message to be printed. Might be NULL pointer.
 * @param ex Termination of program flag (exit if true). Defined constants of 'err.h' header might be used for this
 * value.
 */
void err_handle(char const *const msg, int const ex);

// END GUARD
#endif