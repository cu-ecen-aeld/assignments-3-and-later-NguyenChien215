#!/bin/bash

# Check if the number of arguments is correct
if [ "$#" -ne 2 ]; then
    echo "Error: Two arguments are required."
    exit 1
fi

# Extract the arguments
writefile=$1
writestr=$2

# Check if writefile is specified
if [ -z ${writefile} ]; then
    echo "Error: The file path is not specified."
    exit 1
fi

# Check if writestr is specified
if [ -z "$writestr" ]; then
    echo "Error: The text string to write is not specified."
    exit 1
fi

# Create the directory path if it doesn't exist
mkdir -p "$(dirname "$writefile")"

# Write the content to the file
echo "$writestr" > "$writefile"

# Check if the file was created successfully
if [ $? -ne 0 ]; then
    echo "Error: Failed to create the file."
    exit 1
fi

echo "File '$writefile' created successfully with content '$writestr'."
exit 0
