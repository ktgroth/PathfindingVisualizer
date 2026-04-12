#!/bin/bash

CC="gcc"
LD="gcc"

CFLAGS="-O2 -Wall -Wextra -g -Iinclude -I/usr/include/freetype2 -I/usr/include/libpng16"
LFLAGS="-lglfw -lGLEW -lGL -lfreetype"

SRC=src
OBJ=obj
BUILD=build

SRCS=($(find "$SRC" -name "*.c"))
OBJS=()

for src in "${SRCS[@]}"; do
    obj="${OBJ}/$(basename "$src" .c).o"
    OBJS+=("$obj")
done

OUTPUT=$BUILD/fd

function build {
    clean

    mkdir -p $OBJ
    mkdir -p $BUILD

    for i in "${!SRCS[@]}"; do
        src="${SRCS[$i]}"
        obj="${OBJS[$i]}"
        comp $src $obj
    done

    $LD $LFLAGS "${OBJS[@]}" -o $OUTPUT -lm
}

function comp {
    $CC $CFLAGS -c $1 -o $2
}

function run {
    if [ ! -e "$OUTPUT" ]; then
        build
    fi
    ./$OUTPUT
}

function clean {
    clear

    rm -rf $OBJ
    rm -rf $BUILD
}

case "${1:-build}" in
    build) build ;;
    run) run ;;
    clean) clean ;;
    *) echo "Usage: $0 [build|run|clean]"; exit 1 ;;
esac

