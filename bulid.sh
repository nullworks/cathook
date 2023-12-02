#!/usr/bin/env bash

pushd external/
if [ ! -e boost-cmake ]; then
git clone https://github.com/STEVE4git/boost-cmake.git
fi
if [ ! -e source-sdk-2013-headers ]; then
git clone https://github.com//RosneBurgerworks/source-sdk-2013-headers
fi
if [ ! -e libxoverlay ]; then
git clone https://github.com//RosneBurgerworks/libxoverlay
fi
if [ ! -e TF2_NavFile_Reader ]; then
git clone https://github.com/nullworks/TF2_NavFile_Reader
fi
if [ ! -e clip ]; then
git clone https://github.com/nullworks/clip
fi
if [ ! -e MicroPather ]; then
git clone https://github.com/explowz/MicroPather
fi
if [ ! -e simple-ipc ]; then
git clone https://github.com/RosneBurgerworks/simple-ipc.git
fi
popd

mkdir -p ./obj
# Update cathook
pushd obj && cmake .. && cmake --build . --target cathook -- -j"$(nproc --all)" || {
echo -e "\033[1;31m \n \nFailed to compile cathook\n\033[0m"
exit 1
}
# Update data
cwd="$(pwd)"
cmake --build . --target data || {
echo -e "\033[1;31m\nFailed to update /opt/cathook/data directory! Trying with root rights!\n\033[0m"
sudo bash -c "cd \"$cwd\"; cmake --build . --target data" || {
    echo -e "\033[1;31m\nFailed to update /opt/cathook/data directory\n\033[0m"
    exit 1
    }
}
popd
echo -e "\n\033[1;34mcowhook updated successfully\n\033[0m"

read -r -p "Remove /external extras? [y/N] " response
if [[ "$response" =~ ^([yY][eE][sS]|[yY])$ ]]
then

pushd external/
rm -rf boost-cmake
rm -rf source-sdk-2013-headers
rm -rf libxoverlay
rm -rf TF2_NavFile_Reader
rm -rf clip
rm -rf MicroPather
rm -rf simple-ipc
popd

fi