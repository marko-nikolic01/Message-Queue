#include "message_queue_system.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void initMessageQueueSystem(MessageQueueSystem *mq) {
    mq->channel_count = 0;
    pthread_mutex_init(&mq->mutex, NULL);
}

int findChannel(MessageQueueSystem *mq, const char *channel_name) {
    for (int i = 0; i < mq->channel_count; i++) {
        if (strcmp(mq->channels[i].name, channel_name) == 0) {
            return i;
        }
    }
    return -1;
}
