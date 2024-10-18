#include "message_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void initQueue(MessageQueue *queue) {
    queue->messages = (Message **)malloc(sizeof(Message *) * MAX_QUEUE_SIZE);
    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;
    pthread_mutex_init(&queue->mutex, NULL);
}

