/**
 * @Author Andrej Pavlovič
 * @Email <xpavlo14@vutbr.cz>
 * @Project DNS Tunneling
 * @Program Events handling
 */

#include "events.h"

void event_init(struct event *const event) {
    event->fileSize = event->chunkId = 0;
    event->active = INACTIVE;
}