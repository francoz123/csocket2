#!/bin/bash

# Check if two arguments are provided
if [ $# != 1 ]; then
    echo "Usage: $0 <port>"
    exit 1
fi

# Call the C program and pass the arguments
./server $1
