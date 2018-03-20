#!/bin/bash

cur_dir=$PWD
build_dir=bin/lin

# build cmake
mkdir -p $build_dir
cd $build_dir

# TODO: fix ;(
if [ ! -e "$(realpath ./MakeFile)" ]; then
  echo kms
fi

if [ ! $# -eq 0 ] || [ ! -f ./MakeFile ]; then
  cmake $cur_dir $@ # pass the args to cmake
fi

# make  with all threads
make -j$(nproc)

cd $cur_dir

ln -sf "$build_dir/libnekohook.so" ./
