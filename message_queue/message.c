#include "message.h"
#include <stdlib.h>
#include <string.h>

Message* createMessage(const char *content, int id) {
    Message *msg = (Message *)malloc(sizeof(Message));
    msg->content = strdup(content);
    msg->id = id;
    return msg;
}

void freeMessage(Message *msg) {
    if (msg) {
        free(msg->content);
        free(msg);
    }
}
