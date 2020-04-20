#!/usr/bin/env bash

set -e

BOOST_VERSION=1.67.0

function finish {
  rm -rf ${tmp_dir}
}
trap finish EXIT

out_dir=$(pwd)
patch_dir=$(pwd)/patch/${BOOST_VERSION}
tmp_dir=$(mktemp -d)

echo "Downloading Boost ${BOOST_VERSION}..."
curl -L "https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION//\./_}.tar.bz2" > ${tmp_dir}/boost_${BOOST_VERSION}.tar.bz2

mkdir -p ${tmp_dir}/extract
cd ${tmp_dir}/extract
echo "Extracting archive..."
tar xf ${tmp_dir}/boost_${BOOST_VERSION}.tar.bz2

cd boost_*

if [ -d "${patch_dir}" ]; then
  mkdir patch
  for f in ${patch_dir}/*.patch; do
    echo "Applying patch ${f}..."
    git apply --verbose $f
    cp $f patch/
  done
fi

echo "Removing extra files..."
find . -name "doc" -print0 | xargs -0 -- rm -rf
find . -name "*.htm*" -delete
find . -name "*.png" -delete
find . -name "*.bmp" -delete
find . -name "*.jpg" -delete

cd ..
echo "Recompressing archive..."
tar cfJ ${out_dir}/boost_${BOOST_VERSION//./_}.tar.xz boost_*
