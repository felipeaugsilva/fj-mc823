#!/bin/bash

echo "Removing old binaries..."
test -f client && rm -f client
test -f server && rm -f server
echo "Compiling client_echo.sh..."
gcc -Wall client_echo.c -o client
echo "Compiling server_echo.sh..."
gcc -Wall server_echo.c -o server
echo "Compilation finished!"
