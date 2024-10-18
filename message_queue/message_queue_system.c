#include "message_queue_system.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void initMessageQueueSystem(MessageQueueSystem *mq) {
    mq->channel_count = 0;
    pthread_mutex_init(&mq->mutex, NULL);
}

int findChannel(MessageQueueSystem *mq, const char *channel_name) {
    for (int i = 0; i < mq->channel_count; i++) {
        if (strcmp(mq->channels[i].name, channel_name) == 0) {
            return i;
        }
    }
    return -1;
}

void handleClient(int client_socket, MessageQueueSystem *mq) {
    char buffer[1024];
    int n;

    while ((n = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[n] = '\0';
        char command[20], channel_name[50], message_content[256];
        sscanf(buffer, "%[^:]:%[^:]:%[^\n]", command, channel_name, message_content);

        int channel_index = findChannel(mq, channel_name);
        if (channel_index == -1) {
            pthread_mutex_lock(&mq->mutex);
            initChannel(&mq->channels[mq->channel_count], channel_name);
            mq->channel_count++;
            pthread_mutex_unlock(&mq->mutex);
            channel_index = mq->channel_count - 1;
        }

        Channel *channel = &mq->channels[channel_index];

        if (strcmp(command, "ENQUEUE") == 0) {
            if (enqueue(&channel->queue, message_content) == 0) {
                send(client_socket, "Message enqueued\n", 18, 0);
            } else {
                send(client_socket, "Queue is full\n", 15, 0);
            }
        } else if (strcmp(command, "DEQUEUE") == 0) {
            Message *msg = dequeue(&channel->queue);
            if (msg) {
                send(client_socket, msg->content, strlen(msg->content), 0);
                acknowledge(&channel->queue);
                freeMessage(msg);
            } else {
                send(client_socket, "Queue is empty\n", 16, 0);
            }
        } else if (strcmp(command, "COUNT") == 0) {
            int count = count(&channel->queue);
            snprintf(buffer, sizeof(buffer), "Count: %d\n", count);
            send(client_socket, buffer, strlen(buffer), 0);
        } else if (strcmp(command, "LIST") == 0) {
            pthread_mutex_lock(&channel->queue.mutex);
            for (int i = 0; i < channel->queue.count; i++) {
                Message *msg = channel->queue.messages[(channel->queue.front + i) % MAX_QUEUE_SIZE];
                snprintf(buffer, sizeof(buffer), "Message ID: %d, Content: %s\n", msg->id, msg->content);
                send(client_socket, buffer, strlen(buffer), 0);
            }
            pthread_mutex_unlock(&channel->queue.mutex);
        }
    }
    close(client_socket);
}

void startServer(MessageQueueSystem *mq, int port) {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("accept failed");
            continue;
        }
        pthread_t thread;
        pthread_create(&thread, NULL, (void *)handleClient, (void *)(intptr_t)client_socket);
        pthread_detach(thread);
    }
}
