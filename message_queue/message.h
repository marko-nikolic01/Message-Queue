#ifndef MESSAGE_H
#define MESSAGE_H

typedef struct Message {
    char *content;
    int id;
} Message;

Message* createMessage(const char *content, int id);
void freeMessage(Message *msg);

#endif // MESSAGE_H
