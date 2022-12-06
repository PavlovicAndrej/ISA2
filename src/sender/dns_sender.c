/**
 * @Author Andrej Pavlovič
 * @Email <xpavlo14@vutbr.cz>
 * @Project DNS Tunneling
 * @Program Client implementation
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../common/base16.h"
#include "../common/err.h"
#include "../common/definitions.h"
#include "../common/arguments.h"
#include "dns_sender_events.h"
#include "../common/events.h"

/**
 * Runs client and transfer file to server.
 *
 * @param UPSTREAM_DNS_IP Upstream DNS IP program argument.
 * @param BASE_HOST Base host of server program argument.
 * @param DST_FILEPATH Destination filepath program argument.
 * @param SRC_FILEPATH Source filepath program argument.
 * @param MILLISECONDS Milliseconds program argument.
 */
void client(char *const UPSTREAM_DNS_IP, char *const BASE_HOST, char *const DST_FILEPATH, char *const SRC_FILEPATH, char *const MILLISECONDS);

/**
 * Parses arguments of program and sets their addresses to the passed pointers (or default values for not present
 * optional arguments).
 *
 * @param argc 'argc' passed to 'main()' function.
 * @param argv 'argv' passed to 'main()' function.
 * @param UPSTREAM_DNS_IP Pointer to which save UPSTREAM_DNS_IP optional argument.
 * @param BASE_HOST Pointer to which save BASE_HOST positional argument.
 * @param DST_FILEPATH Pointer to which save DST_FILEPATH positional argument.
 * @param SRC_FILEPATH Pointer to which save SRC_FILEPATH optional positional argument.
 */
void arg_parse(int const argc, char *const argv[], char **const UPSTREAM_DNS_IP, char **const BASE_HOST, char **const DST_FILEPATH, char **const SRC_FILEPATH, char **const MILLISECONDS);

/**
 * Checks, if values of passed program arguments by user are valid.
 *
 * @param UPSTREAM_DNS_IP Upstream DNS IP program argument.
 * @param BASE_HOST Base host of server program argument.
 * @param MILLISECONDS Milliseconds program argument.
 */
void arg_check(char const *const UPSTREAM_DNS_IP, char const *const BASE_HOST, char const *const MILLISECONDS);

/**
 * Encodes name section of DNS question into valid DNS coding (www.google.com -> 3www6google3com0)
 *
 * @param buf Destination buffer.
 * @param str Source string.
 */
void name_encode(char *const buf, char const *const str);

/**
 * Puts data into DNS valid packet.
 *
 * DNS ID is set to client process ID. Recursion desired flag is set to true.
 *
 * @param data Raw data (possibly data chunk) to be encoded into DNS packet.
 * @param data_len Raw data's length in bytes.
 * @param BASE_HOST Base host of server program argument.
 * @param buf Buffer to which output DNS packet is constructed.
 * @return
 */
short build_dns_packet(char const *const data, short const data_len, char *const BASE_HOST, char *const buf);

/**
 * Get configured default name servers of system and save them into array of strings 'name_servers'. If
 * 'UPSTREAM_DNS_IP' is not NULL, copy it into 'name_servers', otherwise, parse IP addresses from /etc/resolv.conf and
 * copy those.
 *
 * @param UPSTREAM_DNS_IP Upstream DNS IP program argument.
 * @param name_servers Character 2D array into which parsed server addresses will be saved.
 * @return Number of found addresses.
 */
short get_default_name_servers(char const *const UPSTREAM_DNS_IP, char name_servers[MAX_NAME_SERVERS][MAX_IPv4_LENGTH]);


// Initialize event data structure globally, so it doesn't have to be passed to every function
struct event event;


int main(int const argc, char *const argv[]) {
    // Parse and check program arguments
    char *UPSTREAM_DNS_IP, *BASE_HOST, *DST_FILEPATH, *SRC_FILEPATH, *MILLISECONDS;
    arg_parse(argc, argv, &UPSTREAM_DNS_IP, &BASE_HOST, &DST_FILEPATH, &SRC_FILEPATH, &MILLISECONDS);
    arg_check(UPSTREAM_DNS_IP, BASE_HOST, MILLISECONDS);

    // Run client
    client(UPSTREAM_DNS_IP, BASE_HOST, DST_FILEPATH, SRC_FILEPATH, MILLISECONDS);

    return 0;
}

void client(char *const UPSTREAM_DNS_IP, char *const BASE_HOST, char *const DST_FILEPATH, char *const SRC_FILEPATH, char *const MILLISECONDS) {
    char dns[DNS_MAX_PACKET]; // DNS packet buffer
    char chunk[(DNS_MAX_NAME - strlen(BASE_HOST) - MAX_DOTS) / 2]; // data buffer (2 stands for b16 encoding overhead)
    int sockfd;
    int chunk_len;
    struct sockaddr_in servaddr;
    FILE *file;

    // Initialize event
    event_init(&event);
    event.filePath = DST_FILEPATH;

    char name_servers[MAX_NAME_SERVERS][MAX_IPv4_LENGTH];
    int name_servers_count = get_default_name_servers(UPSTREAM_DNS_IP, name_servers);

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        err_handle("socket creation failed", EXIT);
    }

    // Set timeout for sending
    struct timeval timeout;
    timeout.tv_sec = 6;
    timeout.tv_usec = 0;
    if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout,sizeof(timeout)) < 0) {
        err_handle("set timeout option of socket failed", WARNING);
    }

    // Filling DNS server information
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);

    // Connect the client socket to DNS server socket
    if (!name_servers_count) {
        err_handle("DNS server is not configured locally, nor set by upstream '-u' option", EXIT);
    }
    int connect_return = -1; // status returned by connect() function
    for (int i = 0; i < name_servers_count; i++) {
        servaddr.sin_addr.s_addr = inet_addr(name_servers[i]);
        if ((connect_return = connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr))) == 0) {
            event.addr = (struct in_addr *) &servaddr.sin_addr.s_addr;
            break;
        }
    }
    if (connect_return == -1) {
        err_handle("unable to connect to DNS server(s)", EXIT);
    }

    // Open file to stream to server
    if (SRC_FILEPATH) {
        if (!(file = fopen(SRC_FILEPATH, "rb"))) {
            err_handle("failed to open file for read", EXIT);
        }
    } else {
        file = stdin;
    }

    // Transfer path to server
    short dns_len = build_dns_packet(DST_FILEPATH, strlen(DST_FILEPATH), BASE_HOST, dns);
    if (write(sockfd, dns, dns_len) != dns_len) {
        err_handle("unable to send data (write on socket)", EXIT);
    }

    // Transfer file to server
    event.active = ACTIVE;
    dns_sender__on_transfer_init(event.addr);
    while ( (chunk_len = fread(chunk, 1, sizeof(chunk), file)) ) {
        // Transfer one chunk
        dns_len = build_dns_packet(chunk, chunk_len, BASE_HOST, dns);
        if (write(sockfd, dns, dns_len) != dns_len) {
            dns_sender__on_transfer_completed(event.filePath, event.fileSize);
            err_handle("unable to send data (write on socket)", EXIT);
        }
        dns_sender__on_chunk_sent(event.addr, event.filePath, event.chunkId, chunk_len);
        event.fileSize += chunk_len;
        event.chunkId++;
    }
    if (!feof(file)) { // Check for fread() errors
        close(sockfd);
        fclose(file);
        dns_sender__on_transfer_completed(event.filePath, event.fileSize);
        err_handle("could not finish reading of file", EXIT);
    }

    // Sleep process before closing connection, so, if DNS server is recursive, it has enough time to send data further
    if (usleep(strtol(MILLISECONDS, NULL, 10) * 1000)) {
        err_handle("unable to sleep process before closing TCP connection (file might have not been received whole by receiver)", WARNING);
    }

    // Clean
    close(sockfd);
    fclose(file);
    dns_sender__on_transfer_completed(event.filePath, event.fileSize);
}

void arg_parse(int const argc, char *const argv[], char **const UPSTREAM_DNS_IP, char **const BASE_HOST, char **const DST_FILEPATH, char **const SRC_FILEPATH, char **const MILLISECONDS) {
    int err_flag = 0;
    char opt;
    opterr = 0; // mute getopt()'s stderr output global flag

    // Pre-initialize optional arguments
    *UPSTREAM_DNS_IP = NULL;
    *MILLISECONDS = "1000";

    // Options
    while ((opt = getopt(argc, argv, "u:s:")) != -1) {
        switch (opt) {
            case 'u':
                *UPSTREAM_DNS_IP = optarg;
                break;
            case 's':
                *MILLISECONDS = optarg;
                break;
            default:
                err_flag++;
        }
    }

    // Positional arguments
    if (optind >= argc) {
        err_flag++;
    } else {
        *BASE_HOST = argv[optind++];
    }
    if (optind >= argc) {
        err_flag++;
    } else {
        *DST_FILEPATH = argv[optind++];
    }
    if (optind >= argc) {
        *SRC_FILEPATH = NULL;
    } else {
        *SRC_FILEPATH = argv[optind++];
    }
    if (optind < argc) {
        err_flag++;
    }

    if (err_flag) {
        char const *const msg = "Usage: dns_sender [options] BASE_HOST DST_FILEPATH [SRC_FILEPATH]\n\nOptions:\n-u UPSTREAM_DNS_IP\tforcing address of remote DNS server\n-s MILLISECONDS\t\tsleep process before closing TCP connection, integer, >=0, default(1000)";
        err_handle(msg, EXIT);
    }
}

void arg_check(char const *const UPSTREAM_DNS_IP, char const *const BASE_HOST, char const *const MILLISECONDS) {
    // Check dns ip (optional)
    if (UPSTREAM_DNS_IP) {
        struct sockaddr_in sa;
        if (inet_pton(AF_INET, UPSTREAM_DNS_IP, &(sa.sin_addr)) == 0) {
            err_handle("DNS upstream ip address is invalid", EXIT);
        }
    }

    // Check milliseconds (optional)
    if (MILLISECONDS) {
        for (int i = 0; i < strlen(MILLISECONDS); i++) {
            if (!(*(MILLISECONDS + i) >= '0' && *(MILLISECONDS + i) <= '9')) {
                err_handle("invalid sleep time", EXIT);
            }
        }
    }

    // Check base host (positional)
    check_host_lex(BASE_HOST);
}

void name_encode(char *const buf, char const *const str) {
    unsigned char cnt = -1; // do not count (but do copy) also last zero terminating character

    for (int i = strlen(str); i >= 0; i--) {
        if (*(str+i) == '.') {
            *(buf+i+1) = (char) cnt;
            cnt = 0;
        } else {
            *(buf+i+1) = *(str+i);
            cnt++;
        }
    }

    *buf = (char) cnt;
}

short build_dns_packet(char const *const data, short const data_len, char *const BASE_HOST, char *const buf) {
    unsigned short offset = 0;

    // Leave space for packet length (which is required when sending DNS over TCP)
    offset += DNS_TCP;

    // Append header
    struct dns_header header;
    memset(&header, 0, sizeof(header));
    header.id = htons(getpid());
    header.rd = 1;
    header.q_count = htons(1);
    memcpy(buf + offset, &header, sizeof(struct dns_header));
    offset += sizeof(struct dns_header);

    /* Append name of question (encode data with base16, split them with dots by maximum label lengths, append base to
     * them and encode whole string into DNS format with replaced dots)
     * Example of effect of the following code: (data : "##") and (base : "example.com") will on buffer copy string
     * '4CDCD7example3com0' ('#' base16 encoded = 'CD') */
    unsigned data_b16_en_len = data_len * 2;
    char name[DNS_MAX_NAME];
    char data_b16_en[data_b16_en_len];
    b16_encode(data_b16_en, data, data_len);
    // Copies data from 'data_b16_en' to 'name' and insert dot character between labels (except last label)
    int i;
    for (i = 0; i < data_b16_en_len / DNS_MAX_LABEL; i++) {
        strncpy(name + i * DNS_MAX_LABEL + i, data_b16_en + i * DNS_MAX_LABEL, DNS_MAX_LABEL);
        *(name + (i + 1) * DNS_MAX_LABEL + i) = '.';
    }
    // Do last label
    strncpy(name + i * DNS_MAX_LABEL + i, data_b16_en + i * DNS_MAX_LABEL, data_b16_en_len % DNS_MAX_LABEL);
    if (data_b16_en_len % DNS_MAX_LABEL) {
        *(name + i * DNS_MAX_LABEL + i + data_b16_en_len % DNS_MAX_LABEL) = '.';
        *(name + i * DNS_MAX_LABEL + i + data_b16_en_len % DNS_MAX_LABEL + 1) = '\0';
    } else {
        *(name + i * DNS_MAX_LABEL + i + data_b16_en_len % DNS_MAX_LABEL) = '\0';
    }
    // Base
    strcat(name, BASE_HOST);
    if (event.active) {
        dns_sender__on_chunk_encoded(event.filePath, event.chunkId, name);
    }
    // Finally, encode into DNS packet format and copy into buffer
    name_encode(buf + offset, name);
    offset += strlen(name);
    offset++; // DNS packet format encoding adds one extra character
    offset++; // Append name's finishing zero byte, but it's already zero (string terminated), so just increment

    // Append tail
    struct dns_question_tail tail;
    tail.type = htons(1);
    tail.class = htons(1);
    memcpy(buf + offset, &tail, sizeof(struct dns_question_tail));
    offset += sizeof(struct dns_question_tail);

    // Fill left space for packet length
    *((unsigned short *) buf) = htons(offset - DNS_TCP);

    return offset;
}

short get_default_name_servers(char const *const UPSTREAM_DNS_IP, char name_servers[MAX_NAME_SERVERS][MAX_IPv4_LENGTH]) {
    if (UPSTREAM_DNS_IP) {
        strcpy(*name_servers, UPSTREAM_DNS_IP);
        return 1;
    }

    size_t i = 0, line_len;
    FILE *stream;
    char *line = NULL;

    if (!(stream = fopen("/etc/resolv.conf", "r"))) {
        err_handle("failed to open file \"/etc/resolv.conf\" to get local default name servers", EXIT);
    }

    while ((line_len = getline(&line, &line_len, stream)) != -1 && i < MAX_NAME_SERVERS) {
        if (strlen(line) >= 11 && strncmp(line, "nameserver ", 11) == 0) { // 11 -> strlen("nameserver ")
            if (line[line_len - 1] == '\n') {
                line[line_len - 1] = '\0';
            }
            strcpy(*(name_servers + i++), line + 11);
        }
    }

    if (line) {
        free(line);
    }
    fclose(stream);

    return i;
}