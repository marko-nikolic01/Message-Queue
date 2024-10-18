#include "message_queue_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_QUEUE_SIZE 100

typedef struct {
    int client_socket;
    MessageQueueSystem *mq;
} ClientArgs;

void initMessageQueueSystem(MessageQueueSystem *mq) {
    mq->channel_count = 0;
    if (pthread_mutex_init(&mq->queueMutex, NULL) != 0) {
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }
}

int findChannel(MessageQueueSystem *mq, const char *channel_name) {
    for (int i = 0; i < mq->channel_count; i++) {
        if (strcmp(mq->channels[i].name, channel_name) == 0) {
            return i;
        }
    }
    return -1;
}

void handleClient(void *arg) {
    ClientArgs *args = (ClientArgs *)arg;
    int client_socket = args->client_socket;
    MessageQueueSystem *mq = args->mq;

    char buffer[1024];
    int n;

    while ((n = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        char command[20], channel_name[50], message_content[256];
        int message_id;
        int parsed = sscanf(buffer, "%[^:]:%[^:]:%[^\n]", command, channel_name, message_content);

        printf("Raw command received: %s\n", buffer);
        fflush(stdout);

        if (parsed < 2) {
            send(client_socket, "Invalid command format\n", 23, 0);
            continue;
        }

        printf("Parsed command: %s, channel: %s\n", command, channel_name);
        if (parsed == 3) {
            printf("Message content: %s\n", message_content);
        } else {
            printf("No message content provided.\n");
        }
        fflush(stdout);

        pthread_mutex_lock(&mq->queueMutex);
        int channel_index = findChannel(mq, channel_name);
        if (channel_index == -1) {
            if (mq->channel_count < MAX_CHANNELS) {
                initChannel(&mq->channels[mq->channel_count], channel_name);
                channel_index = mq->channel_count;
                mq->channel_count++;
                printf("Initialized new channel: %s (index: %d)\n", channel_name, channel_index);
            } else {
                pthread_mutex_unlock(&mq->queueMutex);
                send(client_socket, "Maximum channels reached\n", 25, 0);
                continue;
            }
        } else {
            printf("Found existing channel at index: %d\n", channel_index);
        }
        pthread_mutex_unlock(&mq->queueMutex);

        Channel *channel = &mq->channels[channel_index];

        if (strcmp(command, "ENQUEUE") == 0) {
            printf("Processing ENQUEUE for channel: %s\n", channel_name);
            if (enqueue(&channel->channelQueue, message_content) == 0) {
                send(client_socket, "Message enqueued\n", 18, 0);
                printf("Message enqueued successfully: %s\n", message_content);
            } else {
                send(client_socket, "Queue is full\n", 15, 0);
                printf("Failed to enqueue message: Queue is full\n");
            }
        } else if (strcmp(command, "DEQUEUE") == 0) {
            printf("Processing DEQUEUE for channel: %s\n", channel_name);
            Message *msg = dequeue(&channel->channelQueue);

            if (msg) {
                send(client_socket, msg->content, strlen(msg->content), 0);
                printf("Message dequeued and sent: %s\n", msg->content);
            } else {
                send(client_socket, "Queue is empty\n", 16, 0);
                printf("Failed to dequeue message: Queue is empty\n");
            }
            fflush(stdout);
        } else if (strcmp(command, "PEEK") == 0) {
            printf("Processing PEEK for channel: %s\n", channel_name);
            Message *msg = peek(&channel->channelQueue);

            if (msg) {
                send(client_socket, msg->content, strlen(msg->content), 0);
                printf("Message peeked and sent: %s\n", msg->content);
            } else {
                send(client_socket, "Queue is empty\n", 16, 0);
                printf("Failed to peek message: Queue is empty\n");
            }
            fflush(stdout);
        } else if (strcmp(command, "ACKNOWLEDGE") == 0) {
            printf("Processing ACKNOWLEDGE for channel: %s\n", channel_name);

            if (peek(&channel->channelQueue)) {
                acknowledge(&channel->channelQueue);
                send(client_socket, "Message acknowledged\n", 22, 0);
                printf("Message acknowledged\n");
            } else {
                send(client_socket, "Queue is empty\n", 16, 0);
                printf("Failed to acknowledge message: Queue is empty\n");
            }
            fflush(stdout);
        } else if (strcmp(command, "COUNT") == 0) {
            printf("Processing COUNT for channel: %s\n", channel_name);
            int count = countMessages(&channel->channelQueue);
            snprintf(buffer, sizeof(buffer), "Count: %d\n", count);
            send(client_socket, buffer, strlen(buffer), 0);
        } else if (strcmp(command, "LIST") == 0) {
            pthread_mutex_lock(&channel->channelQueue.queueMutex);
            printf("Processing LIST for channel: %s\n", channel_name);
            for (int i = 0; i < channel->channelQueue.count; i++) {
                Message *msg = channel->channelQueue.messages[(channel->channelQueue.front + i) % MAX_QUEUE_SIZE];
                snprintf(buffer, sizeof(buffer), "Message ID: %d, Content: %s\n", msg->id, msg->content);
                send(client_socket, buffer, strlen(buffer), 0);
            }
            pthread_mutex_unlock(&channel->channelQueue.queueMutex);
            printf("Completed listing messages for channel: %s\n", channel_name);
        } else if (strcmp(command, "DELETE") == 0) {
            if (sscanf(message_content, "%d", &message_id) != 1) {
                send(client_socket, "Invalid message ID format\n", 27, 0);
            } else {
                printf("Processing DELETE for message ID: %d\n", message_id);
                if (deleteMessage(&channel->channelQueue, message_id) == 0) {
                    send(client_socket, "Message deleted\n", 16, 0);
                    printf("Message with ID %d deleted successfully\n", message_id);
                } else {
                    send(client_socket, "Message ID not found\n", 21, 0);
                    printf("Failed to delete message: ID %d not found\n", message_id);
                }
            }
            fflush(stdout);
        }
    }
    close(client_socket);
    free(args);
    printf("Client disconnected.\n");
    fflush(stdout);
}

void startServer(MessageQueueSystem *mq, int port) {
    printf("Starting message queue server...\n");
    fflush(stdout);

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

    printf("Server running on port 8080...\n");
    fflush(stdout);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("accept failed");
            continue;
        }

        ClientArgs *args = malloc(sizeof(ClientArgs));
        args->client_socket = client_socket;
        args->mq = mq;

        pthread_t thread;
        pthread_create(&thread, NULL, (void *(*)(void *))handleClient, (void *)args);
        pthread_detach(thread);
    }
}
