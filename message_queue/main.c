#include "message_queue_system.h"

int main() {
    MessageQueueSystem mq;
    initMessageQueueSystem(&mq);
    startServer(&mq, 8080);
    return 0;
}
