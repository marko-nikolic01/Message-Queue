#ifndef MESSAGE_QUEUE_SYSTEM_H
#define MESSAGE_QUEUE_SYSTEM_H

#include "channel.h"

#define MAX_CHANNELS 10

typedef struct MessageQueueSystem {
    Channel channels[MAX_CHANNELS];
    int channel_count;
    pthread_mutex_t mutex;
} MessageQueueSystem;

void initMQSystem(MessageQueueSystem *mq);
int findChannel(MessageQueueSystem *mq, const char *channel_name);
void startServer(MessageQueueSystem *mq, int port);

#endif