#include "message_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_QUEUE_SIZE 100

void initQueue(MessageQueue *queue) {
    queue->messages = (Message **)malloc(sizeof(Message *) * MAX_QUEUE_SIZE);
    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;
    queue->nextId = 0;
    pthread_mutex_init(&queue->queueMutex, NULL);
}

int enqueue(MessageQueue *queue, const char *message_content) {
    pthread_mutex_lock(&queue->queueMutex);
    if (queue->count == MAX_QUEUE_SIZE) {
        pthread_mutex_unlock(&queue->queueMutex);
        return -1;
    }

    Message *msg = createMessage(message_content, queue->nextId);
    queue->messages[queue->rear % MAX_QUEUE_SIZE] = msg;
    queue->rear++;
    queue->count++;
    queue->nextId++;

    pthread_mutex_unlock(&queue->queueMutex);
    return 0;
}

Message* dequeue(MessageQueue *queue) {
    pthread_mutex_lock(&queue->queueMutex);
    if (queue->count == 0) {
        pthread_mutex_unlock(&queue->queueMutex);
        return NULL;
    }

    Message *msg = queue->messages[queue->front % MAX_QUEUE_SIZE];

    queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
    queue->count--;

    pthread_mutex_unlock(&queue->queueMutex);
    return msg;
}

Message* peek(MessageQueue *queue) {
    pthread_mutex_lock(&queue->queueMutex);
    if (queue->count == 0) {
        pthread_mutex_unlock(&queue->queueMutex);
        return NULL;
    }

    Message *msg = queue->messages[queue->front % MAX_QUEUE_SIZE];

    pthread_mutex_unlock(&queue->queueMutex);
    return msg;
}

void acknowledge(MessageQueue *queue) {
    pthread_mutex_lock(&queue->queueMutex);
    if (queue->count > 0) {
        freeMessage(queue->messages[queue->front % MAX_QUEUE_SIZE]);
        queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
        queue->count--;
    }
    pthread_mutex_unlock(&queue->queueMutex);
}

int countMessages(MessageQueue *queue) {
    pthread_mutex_lock(&queue->queueMutex);
    int c = queue->count;
    pthread_mutex_unlock(&queue->queueMutex);
    return c;
}

void listMessages(MessageQueue *queue) {
    pthread_mutex_lock(&queue->queueMutex);
    for (int i = 0; i < queue->count; i++) {
        Message *msg = queue->messages[(queue->front + i) % MAX_QUEUE_SIZE];
        printf("Message ID: %d, Content: %s\n", msg->id, msg->content);
    }
    pthread_mutex_unlock(&queue->queueMutex);
}

int deleteMessage(MessageQueue *queue, int id) {
    pthread_mutex_lock(&queue->queueMutex);
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
            pthread_mutex_unlock(&queue->queueMutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&queue->queueMutex);
    return -1;
}

void freeQueue(MessageQueue *queue) {
    while (queue->count > 0) {
        Message *msg = dequeue(queue);
        freeMessage(msg);
    }
    free(queue->messages);
    pthread_mutex_destroy(&queue->queueMutex);
}
