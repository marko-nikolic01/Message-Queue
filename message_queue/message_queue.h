#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "message.h"
#include <pthread.h>

typedef struct MessageQueue {
    Message **messages;
    int front;
    int rear;
    int count;
    int nextId;
    pthread_mutex_t queueMutex;
} MessageQueue;

void initQueue(MessageQueue *queue);
int enqueue(MessageQueue *queue, const char *message_content);
Message* dequeue(MessageQueue *queue);
Message* peek(MessageQueue *queue);
void acknowledge(MessageQueue *queue);
int countMessages(MessageQueue *queue);
void listMessages(MessageQueue *queue);
int deleteMessage(MessageQueue *queue, int id);
void freeQueue(MessageQueue *queue);

#endif // MESSAGE_QUEUE_H
