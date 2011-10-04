#!/bin/bash

echo "Removing old binaries..."
test -f cludp_echo && rm -f cludp_echo
test -f srvudp_echo && rm -f srvudp_echo
echo "Compiling client_echo.sh..."
gcc -Wall cludp_echo.c -o cludp_echo
echo "Compiling server_echo.sh..."
gcc -Wall srvudp_echo.c -o srvudp_echo
echo "Compilation finished!"
