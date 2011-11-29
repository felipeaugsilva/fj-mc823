#!/bin/bash

echo "Removing old binaries..."
test -f myinetd && rm -f myinetd
test -f mydaytime_tcp && rm -f mydaytime_tcp
test -f mydaytime_udp && rm -f mydaytime_udp
test -f server_echo && rm -f server_echo
echo "Compiling myinetd.c ..."
gcc -g -Wall myinetd.c -o myinetd
echo "Compiling server_echo.c ..."
gcc -g -Wall server_echo.c -o server_echo
echo "Compiling mydaytime_tcp.c ..."
gcc -g -Wall mydaytime_tcp.c -o mydaytime_tcp
echo "Compiling mydaytime_udp.c ..."
gcc -g -Wall mydaytime_udp.c -o mydaytime_udp
echo "Compilation finished!"
