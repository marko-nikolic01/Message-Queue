#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "message.h"

typedef struct MessageQueue {
    Message **messages;
    int front;
    int rear;
    int count;
    pthread_mutex_t mutex;
} MessageQueue;

void initQueue(MessageQueue *queue);
int enqueue(MessageQueue *queue, const char *message_content);
Message* dequeue(MessageQueue *queue);
void acknowledge(MessageQueue *queue);
int count(MessageQueue *queue);
void listMessages(MessageQueue *queue);
int deleteMessage(MessageQueue *queue, int id);
void freeQueue(MessageQueue *queue);

#endif
