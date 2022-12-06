/**
 * @Author Andrej Pavlovič
 * @Email <xpavlo14@vutbr.cz>
 * @Project DNS Tunneling
 * @Program Server implementation
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include "../common/base16.h"
#include "../common/err.h"
#include "../common/definitions.h"
#include "../common/arguments.h"
#include "dns_receiver_events.h"
#include "../common/events.h"

/**
 * Opens iterative server listening on port 53.
 *
 * @param BASE_HOST Base host program argument.
 * @param DST_DIRPATH Destination directory path program argument.
 */
void server(char const *const BASE_HOST, char const *const DST_DIRPATH);

/**
 * Accept incoming TCP client connection and saves received file.
 *
 * Timeout for received data from client is 6 seconds.
 *
 * @param sockfd Server socket file descriptor.
 * @param base_len Length if base host argument string.
 * @param DST_DIRPATH Destination directory path program argument.
 */
void accept_client(int const sockfd, short const base_len, char const *const DST_DIRPATH);

/**
 * Parses arguments of program. If invalid, prints help on standard error and exits program.
 *
 * @param argc 'argc' passed to 'main()' function.
 * @param argv 'argv' passed to 'main()' function.
 * @param BASE_HOST Base host program argument.
 * @param DST_DIRPATH Destination directory path program argument.
 */
void arg_parse(int const argc, char const *const argv[], char const **const BASE_HOST, char const **const DST_DIRPATH);

/**
 * Reads one DNS packet from socket 'connfd' TCP stream and save it's contents into 'dns' buffer, without first two
 * bytes representing length of DNS packet.
 *
 * @param connfd Client's socket file descriptor.
 * @param dns Buffer for DNS packet.
 * @return Returns number of bytes written to buffer 'dns', or zero, if there was no data available (meaning FIN flag
 * was received).
 */
short receive_dns_packet(int const connfd, char *const dns);

/**
 * Inline function used to customize read() standard function to exit program with error, if read() returns value '-1'.
 * Except this behaviour is function my_reda() equal to function read(), including return value and parameters. For more
 * info on these check man of read().
 */
static inline short my_read(int const fd, void *const buf, int const nbytes);

/**
 * Extract data from DNS packet.
 *
 * @param dns DNS packet.
 * @param dns_len Length of DNS packet passed in 'dns' parameter.
 * @param base_len Length if base host argument string.
 * @param buf Buffer to which save extracted data from DNS packet.
 * @return Number of bytes extracted from DNS packet.
 */
short disassemble_dns_packet(char const *const dns, short const dns_len, short const base_len, char *const buf);

/**
 * Attempts to create all directories contained in path.
 *
 * @param path_const Path.
 */
void create_dirs(char const *const path);


// Initialize event data structure globally, so it doesn't have to be passed to every function
struct event event;


int main(int const argc, char const *const argv[]) {
    // Parse program arguments
    const char *BASE_HOST, *DST_DIRPATH;
    arg_parse(argc, argv, &BASE_HOST, &DST_DIRPATH);
    check_host_lex(BASE_HOST);

    // Run server
    server(BASE_HOST, DST_DIRPATH);

    return 0;
}

void server(char const *const BASE_HOST, char const *const DST_DIRPATH) {
    int sockfd;
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        err_handle("socket creation failed", EXIT);
    }

    // Filling server information
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Bind
    if ((bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr))) != 0) {
        err_handle("socket bind failed", EXIT);
    }

    // Listen
    if ((listen(sockfd, 1)) != 0) {
        err_handle("listen failed", EXIT);
    }

    // Start accepting incoming connections in infinite loop
    short const base_len = strlen(BASE_HOST);
    for (;;) {
        // Initialize event
        event_init(&event);

        // Accept client
        accept_client(sockfd, base_len, DST_DIRPATH);
    }
}

void accept_client(int const sockfd, short const base_len, char const *const DST_DIRPATH) {
    int connfd;
    char dns[DNS_MAX_PACKET]; // DNS packet buffer
    char chunk[(DNS_MAX_NAME - base_len - MAX_DOTS) / 2]; // 2 stands for b16 encoding overhead
    FILE *file;
    struct sockaddr_in cliaddr;

    // Filling client information
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Accept
    unsigned len = sizeof(cliaddr);
    if ((connfd = accept(sockfd, (struct sockaddr *) &cliaddr, &len)) < 0) {
        err_handle("accept failed", WARNING);
    }
    event.addr = &cliaddr.sin_addr;

    // Set timeout for receiving
    struct timeval timeout;
    timeout.tv_sec = 6;
    timeout.tv_usec = 0;
    if (setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, &timeout,sizeof(timeout)) < 0) {
        err_handle("set timeout option of socket failed", WARNING);
    }

    // Get path (into chunk variable)
    short dns_len = receive_dns_packet(connfd, dns);
    if (!dns_len) {
        close(connfd);
        return;
    }
    short chunk_len = disassemble_dns_packet(dns, dns_len, base_len, chunk);

    // Concatenate paths
    short DST_DIRPATH_len = strlen(DST_DIRPATH);
    char full_path[DST_DIRPATH_len + chunk_len + 2];
    memcpy(full_path, DST_DIRPATH, DST_DIRPATH_len + 1);
    if (full_path[DST_DIRPATH_len - 1] != '/' && *chunk != '/')
        strcat(full_path, "/");
    chunk[chunk_len] = '\0';
    strcat(full_path, chunk);
    event.filePath = full_path;

    // Open (create) file (and possibly directories) for write
    create_dirs(full_path);
    if (!(file = fopen(full_path, "wb"))) {
        char *msg2 = ": failed to open file for write";
        char msg1[sizeof(full_path) + strlen(msg2)];
        strcpy(msg1, full_path);
        strcat(msg1, msg2);
        err_handle(msg1, WARNING);
        close(connfd);
        return;
    }

    // Get and write (save) file
    event.active = ACTIVE;
    dns_receiver__on_transfer_init(event.addr);
    while ((dns_len = receive_dns_packet(connfd, dns))) {
        // Process dns packet and save data
        chunk_len = disassemble_dns_packet(dns, dns_len, base_len, chunk);
        if(fwrite(chunk, 1, chunk_len, file) != chunk_len) { // cannot write to file
            char *msg2 = ": failed to write";
            char msg1[sizeof(full_path) + strlen(msg2)];
            strcpy(msg1, full_path);
            strcat(msg1, msg2);
            err_handle(msg1, WARNING);
            close(connfd);
            fclose(file);
            dns_receiver__on_transfer_completed(event.filePath, event.fileSize);
            return;
        }
        dns_receiver__on_chunk_received(event.addr, event.filePath, event.chunkId, chunk_len);
        event.fileSize += chunk_len;
        event.chunkId++;
    }

    // Clean
    close(connfd);
    fclose(file);
    dns_receiver__on_transfer_completed(event.filePath, event.fileSize);
}

void arg_parse(int const argc, char const *const argv[], char const **const BASE_HOST, char const **const DST_DIRPATH) {
    if (argc != 3) {
        char const *const msg = "Usage: dns_receiver BASE_HOST DST_DIRPATH";
        err_handle(msg, EXIT);
    }
    *BASE_HOST = argv[1];
    *DST_DIRPATH = argv[2];
}

short receive_dns_packet(int const connfd, char *const dns) {
    short bytes_read, dns_len;

    // Read length of dns packet
    if ((bytes_read = my_read(connfd, &dns_len, 2)) == 0) { // connection closed with FIN flag
        return 0;
    } else if (bytes_read == 1) { // only one out of 2 bytes was read
        my_read(connfd, ((char *) &dns_len) + 1, 1); // read second byte of prefixed length
    }
    dns_len = ntohs(dns_len);

    // Read dns packet (without prefixed length)
    bytes_read = my_read(connfd, dns, dns_len);
    if (bytes_read != dns_len) { // DNS packet was not read whole from stream
        short bytes_missing = dns_len - bytes_read;
        while (bytes_missing) { // read whole DNS packet from stream
            bytes_missing -= my_read(connfd, dns + dns_len - bytes_missing, bytes_missing);
        }
    }

    return dns_len;
}

static inline short my_read(int const fd, void *const buf, int const nbytes) {
    short read_ret = read(fd, buf, nbytes);

    if (read_ret == -1) {
        err_handle("cannot read from client socket", EXIT);
    }

    return read_ret;
}

short disassemble_dns_packet(char const *const dns, short const dns_len, short const base_len, char *const buf) {
    // Extract data from packet
    unsigned short dns_encoded_data_len = dns_len - DNS_HEADER - DNS_TAIL - base_len - 2;
    char encoded_data[dns_encoded_data_len + 1];
    encoded_data[dns_encoded_data_len] = '\0';
    memcpy(encoded_data, dns + DNS_HEADER, dns_encoded_data_len);

    // Handle event
    if (event.active) {
        char encoded_data_for_event[dns_encoded_data_len + base_len + 2];
        char n;
        int offset = 0;
        memcpy(encoded_data_for_event, dns + DNS_HEADER, sizeof(encoded_data_for_event));
        while ((n = *(encoded_data_for_event + offset))) {
            encoded_data_for_event[offset] = '.';
            offset += n + 1;
        }
        dns_receiver__on_query_parsed(event.filePath, encoded_data_for_event + 1);
    }

    // DNS decode data (recognize and remove dot character codes) into same buffer
    unsigned char label_len = *encoded_data, data_len = 0, i = 1;
    while (label_len) {
        memmove(encoded_data + data_len, encoded_data + data_len + i, label_len);
        data_len += label_len;
        label_len = *(encoded_data + data_len + i++);
    }

    // Base16 decode
    b16_decode(buf, encoded_data, data_len);

    return data_len / 2;
}

void create_dirs(char const *const path) {
    char path_copy[strlen(path) + 1]; // not constant copy of path_const
    char *cur = path_copy; // cursor

    // Copy
    strcpy(path_copy, path);

    // Current dir notation './path'
    if (*cur == '.' && *(cur + 1) == '/') {
        cur++;
    }

    // Skip initial slash(es)
    for (; *cur == '/'; cur++);

    // Attempt to create directory(ies)
    while (*cur != '\0') {
        if (*(cur++) == '/') {
            char tmp = *cur;
            *cur = '\0';
            if (mkdir(path_copy, 0777) == -1 && errno != EEXIST) {
                char *msg2 = ": cannot create directory";
                char msg1[strlen(path_copy) + strlen(msg2)];
                strcpy(msg1, path_copy);
                strcat(msg1, msg2);
                err_handle(msg1, WARNING);
                return;
            }
            if (errno == EEXIST) {
                errno = 0;
            }
            *cur = tmp;
        }
    }
}