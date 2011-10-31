#!/bin/bash

echo "Removing old binaries..."
test -f client_echo && rm -f client_echo
test -f server_echo_daemon && rm -f server_echo_daemon
echo "Compiling client_echo.c..."
gcc -g -Wall client_echo.c -o client_echo
echo "Compiling server_echo_daemon.c..."
gcc -g -Wall server_echo_daemon.c -o server_echo_daemon
echo "Compilation finished!"
