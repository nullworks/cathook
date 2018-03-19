#!/bin/bash

cur_dir=$PWD
build_dir=bin/lin

# build cmake
mkdir -p $build_dir
cd $build_dir
if [ ! -f CMakeCache.txt ]; then
 cmake $cur_dir $@ # pass the args to cmake
fi

# make  with all threads
make -j$(nproc)

cd $cur_dir

ln -sf "$build_dir/libnekohook.so" ./
