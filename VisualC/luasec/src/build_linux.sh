#!/bin/bash

make clean
make luasocket MYCFLAGS="-I/usr/include/lua5.1"
make linux INCDIR="-I/usr/include/lua5.1 -I."
