# Cathook Training Software
![banner](http://i.imgur.com/w96wdtE.png)
[![pipeline status](https://gitlab.com/nullworks/cathook-ci/badges/master/pipeline.svg)](https://gitlab.com/nullworks/cathook-ci/commits/master)


[cathook announcements channel in telegram](https://t.me/cathook_cheat)

## Risk of VAC detection

The software Might be VAC detected. Only use it on accounts you won't regret getting VAC banned.

## Community
You can chat with other cathook users in [my official telegram group](https://t.me/nullifiedcat).

## Important

Right now cathook isn't in its greatest state, a lot of things may not work/crash the game, please open issues on github using [this page](https://github.com/nullworks/cathook/issues).

## Overview

cathook is a training software designed for Team Fortress 2 for Linux. cathook includes some joke features like

* Backpack.TF API integration with playerlist GUI, allowing you to see players' inventory values
* Ignore Hoovy
* Encrypted chat
* Chance to get manually VAC banned by Valve

and a lot of useful features, including

* Anti Backstab with option to use "No" voice command when spy tries to backstab you
* Extremely customizable spam (you can make spam lines that'll include name of random dead enemy pyro or sniper)
* Follow Bots
* Working crit hack (does not work right now (works right now))

[FULL LIST OF FEATURES HERE](https://github.com/nullworks/cathook/wiki/List-of-features) (list might be outdated)

# INSTALLATION

## Automatic: (Ubuntu based only)
Run in terminal:

* `wget https://raw.githubusercontent.com/nullworks/One-in-all-cathook-install/master/install-all && bash install-all`

## Manual:
You need CMake to build cathook, CMake should take care of dependencies

Install libglez, libxoverlay and simple-ipc

Clone Cathook (`git clone --recursive https://github.com/nullworks/cathook`)

* `cd <name>`
* `mkdir build && cd build`
* `cmake ..`
* `make && sudo make install`
* `cd ..`

Repeat until libglez, libxoverlay and simple-ipc are installed

Install cathook

* `mkdir build && cd build`
* `cmake .. && make`
* `sudo make data`

Make sure to put the required files in ../requirements/lib*, and cathook in ../cathook.

### Outdated (but might be helpful):

You can use gcc-7 for compiling cathook if you add `-e CC=gcc-7 CXX=g++-7` to make command line

Ubuntu gcc6 installation: (check if you have gcc-6 installed already by typing `gcc-6 -v`
```bash
sudo apt update && sudo apt install build-essential software-properties-common -y && sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y && sudo apt update && sudo apt install gcc-snapshot g++-6-multilib gcc-6 g++-6 -y
```

Ubuntu other dependencies installation:

```bash
sudo apt update && sudo apt install git libssl-dev:i386 libboost-all-dev libc6-dev:i386 gdb libsdl2-dev libglew-dev:i386 libfreetype6-dev:i386 -y 
```


Arch gcc6 & dependencies installation:
```bash
sudo pacman -U https://archive.archlinux.org/packages/g/gcc-multilib/gcc-multilib-6.3.1-2-x86_64.pkg.tar.xz https://archive.archlinux.org/packages/g/gcc-libs-multilib/gcc-libs-multilib-6.3.1-2-x86_64.pkg.tar.xz https://archive.archlinux.org/packages/l/lib32-gcc-libs/lib32-gcc-libs-6.3.1-2-x86_64.pkg.tar.xz && sudo cp -r /usr/include/c++/6.3.1/ /tmp/ && sudo pacman -S gdb gdb-common glew1.10 glew lib32-glew1.10 rsync lib62-gcc-libs gcc-libs-multilib gcc-multilib --noconfirm && yes | sudo cp -r  /tmp/6.3.1/ /usr/include/c++/
```

If you don't use Ubuntu or Arch (or if Arch script gets outdated), here's the list of what cathook requires:

* `cmake-qt-gui` (optional, for easy configuring)
* `cmake`
* `gcc-7`
* `g++-7`
* `gcc-7-multilib`
* `g++-7-multilib`
* `glew`
* `gdb` (for the injection script, you can use different injector if you want)
* `libssl-dev:i386`
* `libc6-dev:i386`
* `libsdl2-dev`
* `libglew-dev:i386`
* `libfreetype6-dev:i386`
* `libboost-all-dev`
* `rsync` (used for copying shaders/fonts to tf2 data directory, `check-data` script)


Cathook installation script:
```bash
git clone --recursive https://github.com/nullworks/cathook && cd cathook && bash build-tf2
```

**Errors while installing?**

`/usr/include/c++/5/string:38:28: fatal error: bits/c++config.h: No such file or directory`
You don't have gcc-7-multilib installed correctly.

Anything related to `glez` or `xoverlay`

Install libglez and libxoverlay.

`src/<any file>: fatal error: mathlib/vector.h: No such file or directory`
You didn't download Source SDK. **DO NOT DOWNLOAD CATHOOK USING "DOWNLOAD .ZIP" FROM GITHUB. USE git clone --recursive!**

If you are using another distro, make sure to have required dependencies installed.

## Updating cathook
Run the `update` script in cathook folder.

Cathook requires a special data folder (contains shaders, font files, walkbot paths, etc). This folder is located at `/opt/cathook/data` and is generated automatically when you compile cathook.

## Injection
`sudo ./attach` to attach cathook into TF2. Optionally, you can provide an argument number (0-n - #) to provide the TF2 process ID (for bots).

`sudo ./attach-backtrace` to attach and print backtrace incase TF2 crashes. Some users report that this causes FPS drop in-game. This is recommended to grab a log of what went wrong if Cathook is crashing on you.

## Followbots (outdated)
`cathook-ipc-server` allows you to run and control Followbots to do your evil bidding in-game. The installation for Followbots is quite complex, and will not be covered on this page. Obviously, you must have several user accounts ready to run TF2.  
A guide for Followbots can be found here: [How to setup and use followbots.](https://www.youtube.com/watch?v=kns5-nw7xUg)  
You may also ask someone in our discord server to help you out.

The installation script is as followed:
```bash
git clone --recursive https://github.com/nullworks/cathook-ipc-server && cd cathook-ipc-server && make -j4
```
To run the Followbot server, run `./bin/cathook-ipc-server`. You can also use `./bin/cathook-ipc-server &>/dev/null &` to run it in background.

