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

int enqueue(MessageQueue *queue, const char *message_content) {
    pthread_mutex_lock(&queue->mutex);
    if (queue->count == MAX_QUEUE_SIZE) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    Message *msg = createMessage(message_content, queue->rear);
    queue->messages[queue->rear % MAX_QUEUE_SIZE] = msg;
    queue->rear++;
    queue->count++;

    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

Message* dequeue(MessageQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    if (queue->count == 0) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }

    Message *msg = queue->messages[queue->front % MAX_QUEUE_SIZE];
    queue->front++;
    queue->count--;

    pthread_mutex_unlock(&queue->mutex);
    return msg;
}

void acknowledge(MessageQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    if (queue->count > 0) {
        freeMessage(queue->messages[queue->front % MAX_QUEUE_SIZE]);
        queue->front++;
        queue->count--;
    }
    pthread_mutex_unlock(&queue->mutex);
}


