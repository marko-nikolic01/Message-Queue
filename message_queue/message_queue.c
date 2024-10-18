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

int count(MessageQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    int c = queue->count;
    pthread_mutex_unlock(&queue->mutex);
    return c;
}

void listMessages(MessageQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    for (int i = 0; i < queue->count; i++) {
        Message *msg = queue->messages[(queue->front + i) % MAX_QUEUE_SIZE];
        printf("Message ID: %d, Content: %s\n", msg->id, msg->content);
    }
    pthread_mutex_unlock(&queue->mutex);
}

int deleteMessage(MessageQueue *queue, int id) {
    pthread_mutex_lock(&queue->mutex);
    for (int i = 0; i < queue->count; i++) {
        int index = (queue->front + i) % MAX_QUEUE_SIZE;
        if (queue->messages[index]->id == id) {
            freeMessage(queue->messages[index]);
            for (int j = i; j < queue->count - 1; j++) {
                queue->messages[(queue->front + j) % MAX_QUEUE_SIZE] =
                    queue->messages[(queue->front + j + 1) % MAX_QUEUE_SIZE];
            }
            queue->rear--;
            queue->count--;
            pthread_mutex_unlock(&queue->mutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&queue->mutex);
    return -1;
}

void freeQueue(MessageQueue *queue) {
    while (queue->count > 0) {
        Message *msg = dequeue(queue);
        freeMessage(msg);
    }
    free(queue->messages);
    pthread_mutex_destroy(&queue->mutex);
}
