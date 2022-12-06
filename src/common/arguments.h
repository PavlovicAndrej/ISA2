/**
 * @Author Andrej Pavlovič
 * @Email <xpavlo14@vutbr.cz>
 * @Project DNS Tunneling
 * @Program Functions shared throughout the project, used for program's arguments handling.
 * @details header file
 */

// GUARD
#ifndef ARGUMENTS_H
#define ARGUMENTS_H

/**
 * Checks syntax of domain name.
 *
 * DNS domain syntax:
 * Allowed characters are alphanumeric (A-Za-z0-9), hyphen (-) or dot (.)
 * Maximum length of full domain is 251 (DNS defines 253, but minimum length of transferred data is 2 bytes)
 * Maximum length of label (subdomain separated by dot) is 63
 * Label cannot start, nor end with hyphen
 * Dot cannot follow dot (..) - that would be empty string domain name, which is reserved for root domain name
 *
 * @param host host string
 */
void check_host_lex(const char *host);

// END GUARD
#endif