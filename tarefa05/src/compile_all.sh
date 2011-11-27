#!/bin/bash

echo "Removing old binaries..."
test -f myinetd && rm -f myinetd
test -f mydaytime_tcp && rm -f mydaytime_tcp
test -f mydaytime_udp && rm -f mydaytime_udp
echo "Compiling myinetd.c ..."
gcc -g -Wall myinetd.c -o myinetd
echo "Compiling mydaytime_tcp.c ..."
gcc -g -Wall mydaytime_tcp.c -o mydaytime_tcp
echo "Compiling mydaytime_udp.c ..."
gcc -g -Wall mydaytime_udp.c -o mydaytime_udp
echo "Compilation finished!"
