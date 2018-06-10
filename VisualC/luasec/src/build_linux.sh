#!/bin/bash

make clean
make luasocket MYCFLAGS="-I/usr/include/luajit-2.1 -fPIC"
make linux INCDIR="-I/usr/include/luajit-2.1 -I."
