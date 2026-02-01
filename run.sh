#!/bin/bash

echo "Usuwam pozostale IPC"
ipcrm -a 2>/dev/null

CC="g++"
CFLAGS="-Wall -Wextra -I ./include -pthread -std=c++17"
COMMON_SRC="src/IPC/*.cpp src/Logger.cpp src/Utils.cpp src/IPC.cpp src/Signals.cpp"
OUT_DIR="."

mkdir -p $OUT_DIR

echo "Kompiluje pliki"

$CC $CFLAGS $COMMON_SRC src/PROCESS/Manager.cpp -o $OUT_DIR/manager
echo "Manager"

$CC $CFLAGS $COMMON_SRC src/PROCESS/Cashier.cpp -o $OUT_DIR/cashier
echo "Cashier"

$CC $CFLAGS $COMMON_SRC src/PROCESS/Baker.cpp -o $OUT_DIR/baker
echo "Baker"

$CC $CFLAGS $COMMON_SRC src/PROCESS/Client.cpp -o $OUT_DIR/client
echo "Client"

if [ $? -eq 0 ]; then
    echo "-------------------"
    echo "Kompilacja udana"
    echo "Uruchamiam managera"
    echo "-------------------"
    cd $OUT_DIR
    ./manager
else
    echo "Nie udalo sie zbudowac programu"
    exit 1
fi
