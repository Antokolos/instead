#!/bin/bash

# g++ -I/usr/include/luajit-2.1 -I/path/to/steam/includes -D_LINUX -o luapassing.so -shared luapassing.cpp adapter.cpp libsteam_api.so -fPIC
g++ -I/usr/include/luajit-2.1 -D_LINUX -o luapassing.so -shared luapassing.cpp adapter.cpp -fPIC
