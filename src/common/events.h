/**
 * @Author Andrej Pavlovič
 * @Email <xpavlo14@vutbr.cz>
 * @Project DNS Tunneling
 * @Program Events handling
 * @details header file
 */

// GUARD
#ifndef EVENTS_H
#define EVENTS_H


#define ACTIVE 1
#define INACTIVE 0

/**
 * Used to handle events.
 *
 * If active, events are called when needed, otherwise they are not.
 *
 * For more information see headers:
 *  "src/sender/dns_sender_events.h"
 *  "src/receiver/dns_receiver_events.h"
 */
struct event {
    int active; // ACTIVE/INACTIVE
    char *filePath;
    int fileSize;
    int chunkId;
    struct in_addr *addr;
};

/**
 * Initialize event with initial values and inactive state. Might be used for event reset.
 *
 * @param event Event to be initialized.
 */
void event_init(struct event *const event);

// END GUARD
#endif