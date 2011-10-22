#!/bin/bash

echo "Removing old binaries..."
test -f cludp_echo && rm -f cludp_echo
test -f srvudp_echo && rm -f srvudp_echo
echo "Compiling cludp_echo.c..."
gcc -Wall cludp_echo.c -o cludp_echo
echo "Compiling srvudp_echo.c..."
gcc -Wall srvudp_echo.c -o srvudp_echo
echo "Compiling flood.c..."
gcc -Wall flood.c -o flood
echo "Compilation finished!"
