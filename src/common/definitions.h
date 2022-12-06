/**
 * @Author Andrej Pavlovič
 * @Email <xpavlo14@vutbr.cz>
 * @Project DNS Tunneling
 * @Program Defined macros, constants and data structures used throughout the program
 * @details header file (Pair implementation file doesn't not exist)
 */

// GUARD
#ifndef CONSTANTS_H
#define CONSTANTS_H

/// DNS port number
#define PORT 53

/**
 * Maximum length of DNS packet used by this application.
 *
 * 2 (TCP length) + 12 (header) + 255 (max question name in DNS encoded form) + 4 (question tail) */
#define DNS_MAX_PACKET 273

/// Maximum length of DNS name part of question in its textual form
#define DNS_MAX_NAME 253

/// Maximum length of one label in name part of question
#define DNS_MAX_LABEL 63

/// Length of 'length of DNS data' part of DNS packet when transferring over TCP
#define DNS_TCP 2

/// Length of DNS header
#define DNS_HEADER 12

/// Length of tail of DNS question part (type and class)
#define DNS_TAIL 4

/// Maximum dots inserted to split data into labels
#define MAX_DOTS 4

/// Maximum count of default name servers that client will try to connect to
#define MAX_NAME_SERVERS 10

/// Maximum length of IPv4 address in its textual form (termination byte included)
#define MAX_IPv4_LENGTH 16


/// DNS header struct
struct dns_header {
    unsigned short id; // identification number

    unsigned char rd :1; // recursion desired
    unsigned char tc :1; // truncated message
    unsigned char aa :1; // authoritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag

    unsigned char rcode :4; // response code
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authenticated data
    unsigned char z :1; // its z! reserved
    unsigned char ra :1; // recursion available

    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};

/// DNS rest of Question section after Name
struct dns_question_tail {
    unsigned short type; // dns record type
    unsigned short class; // allows domain names to be used for arbitrary objects
};

// END GUARD
#endif