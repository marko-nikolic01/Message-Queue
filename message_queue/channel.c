#include "channel.h"
#include <string.h>

void initChannel(Channel *channel, const char *name) {
    strcpy(channel->name, name);
    initQueue(&channel->channelQueue);
}
