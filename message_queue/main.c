#include "message_queue_system.h"
#include <stdio.h>

int main() {
    printf("Starting message queue...\n");
    fflush(stdout);
    MessageQueueSystem mq;
    initMessageQueueSystem(&mq);
    startServer(&mq, 8080);
    pthread_mutex_destroy(&mq.queueMutex);
    return 0;
}
