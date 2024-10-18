#!/bin/bash

SERVER="127.0.0.1"
PORT=8080

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <command> <channel_name> [message_content]"
    exit 1
fi

COMMAND=$1
CHANNEL_NAME=$2
MESSAGE_CONTENT=$3

if [ "$COMMAND" == "ENQUEUE" ] && [ -z "$MESSAGE_CONTENT" ]; then
    echo "Message content is required for ENQUEUE command."
    exit 1
fi

# Prepare the command string
if [ "$COMMAND" == "ENQUEUE" ]; then
    CMD="$COMMAND:$CHANNEL_NAME:$MESSAGE_CONTENT"
else
    CMD="$COMMAND:$CHANNEL_NAME:"
fi

# Send the command to the server
echo "$CMD" | nc $SERVER $PORT
