#!/bin/bash

echo "Removing old binaries..."
test -f myinetd && rm -f myinetd
echo "Compiling myinetd.c ..."
gcc -g -Wall myinetd.c -o myinetd
echo "Compilation finished!"
