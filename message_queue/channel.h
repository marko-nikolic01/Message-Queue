#ifndef CHANNEL_H
#define CHANNEL_H

#include "message_queue.h"

typedef struct Channel {
    MessageQueue queue;
    char name[50];
} Channel;

void initChannel(Channel *channel, const char *name);

#endif // CHANNEL_H
