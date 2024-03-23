#!/bin/bash

# Check if two arguments are provided
if [ $# != 2 ]; then
    echo "Usage: $0 <host name> <port>"
    exit 1
fi

# Call the C program and pass the arguments
./client $1 $2
