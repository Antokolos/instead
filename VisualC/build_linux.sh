#!/bin/bash

# cleaning & recreating
rm -rf _dist_
mkdir _dist_

# luafilesystem
cd luafilesystem
make
cp src/lfs.so ./../_dist_
make clean
cd ..

# luapassing
cd luapassing/src
./build_linux.sh
cp luapassing.so ./../../_dist_
rm luapassing.so
cd ../..

# luasocket
cd luasocket
./linux_alt.cmd
cp src/mime-1.0.3.so ./../_dist_/mime.so
cp src/serial.so ./../_dist_
cp src/socket-3.0-rc1.so ./../_dist_/socket.so
cp src/unix.so ./../_dist_
make clean
cd ..

# luasec
cd luasec/src
./build_linux.sh
cd luasocket
make clean
cd ..
cp ssl.so ./../../_dist_
make clean
cd ../..

# lua files
mkdir _dist_/lua
cp luasec/src/ssl.lua _dist_/lua
mkdir _dist_/lua/ssl
cp luasec/src/https.lua _dist_/lua/ssl
cp luasec/src/options.lua _dist_/lua/ssl
cp luasocket/src/ltn12.lua _dist_/lua
cp luasocket/src/mime.lua _dist_/lua
cp luasocket/src/socket.lua _dist_/lua
mkdir _dist_/lua/socket
cp luasocket/src/ftp.lua _dist_/lua/socket
cp luasocket/src/headers.lua _dist_/lua/socket
cp luasocket/src/http.lua _dist_/lua/socket
cp luasocket/src/smtp.lua _dist_/lua/socket
cp luasocket/src/tp.lua _dist_/lua/socket
cp luasocket/src/url.lua _dist_/lua/socket
cp luahelpers/lua/*.lua _dist_/lua

# INSTEAD and dependencies
mkdir _dist_/appdata
mkdir _dist_/games
cp ../src/sdl-instead _dist_
mkdir _dist_/lang
cp ../lang/*.ini _dist_/lang
mkdir _dist_/libs
./cpld.bash _dist_/sdl-instead _dist_/libs
./cpld.bash _dist_/ssl.so _dist_/libs
rm _dist_/libs/ld-linux*
rm _dist_/libs/libc.*
rm _dist_/libs/libdl.*
rm _dist_/libs/libgcc_s.*
rm _dist_/libs/libm.*
rm _dist_/libs/libpthread.*
rm _dist_/libs/librt.*
#rm _dist_/libs/libstdc++.*
cp -r ../stead _dist_/stead
find _dist_/stead -type f -name "CMakeLists.txt" -delete
find _dist_/stead -type f -name "Makefile" -delete
cp -r ../themes _dist_/themes
find _dist_/themes -type f -name "CMakeLists.txt" -delete
find _dist_/themes -type f -name "Makefile" -delete

# run.sh

cat <<EOT >> _dist_/run.sh
#!/bin/sh

PROGRAM_DIRECTORY=\$( cd \$(dirname "\$0") ; pwd -P )
export LD_LIBRARY_PATH="\$LD_LIBRARY_PATH:\$PROGRAM_DIRECTORY/libs"

cd "\$PROGRAM_DIRECTORY"
"./sdl-instead" -fullscreen -standalone -game nlbhub "\$@"
EOT

chmod +x _dist_/run.sh
