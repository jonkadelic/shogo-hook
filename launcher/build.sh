#!/bin/bash

cd $(dirname $(realpath $0))
echo $PWD

GCC=i686-w64-mingw32-gcc
COMPILER_ARGS="-std=gnu23"
LINKER_ARGS=""
LIBRARIES=""
SRC_DIR=src
BUILD_DIR=build
OUT_PATH=${BUILD_DIR}/shogo_hook_launcher.exe

mkdir -p ${BUILD_DIR}/${SRC_DIR}
for filename in src/*.c; do
    echo "${GCC} ${COMPILER_ARGS} -c ${filename} -o ${BUILD_DIR}/${filename}.o"
    ${GCC} ${COMPILER_ARGS} -c ${filename} -o "${BUILD_DIR}/${filename}.o"
done

${GCC} ${LINKER_ARGS} ${BUILD_DIR}/${SRC_DIR}/*.o ${LIBRARIES} -o ${OUT_PATH}
cp ${OUT_PATH} /home/jonkadelic/win7/