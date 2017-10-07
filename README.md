
# Cathook Modular Fork
![banner](http://i.imgur.com/w96wdtE.png)
Not just for tf2 >_>

# TODO
Readme needs to be rewritten

## Discord Server
[Cats Discord Server](https://discord.gg/6a5tzEK)
The discord server isn't the official server but happens to be where I distribute it myself.
## About

Cathook is an extremely modular training software framework designed to work with many games, even games not using the source engine!   

This cathook fork is currently a work in progress but you may request your build at cats discord server if you're a respected member or have donated.   

Cathook is currently undetected!   

# INSTALLATION

These dependancy install scripts are out of date. Please wait for an update

Ubuntu gcc6 installation: (check if you have gcc-6 installed already by typing `gcc-6 -v`
```bash
sudo apt update && sudo apt install build-essential software-properties-common -y && 
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y && sudo apt update && 
sudo apt install gcc-snapshot g++-6-multilib gcc-6 g++-6 -y
```

Ubuntu other dependencies installation:
```bash
sudo apt update && sudo apt install git libssl-dev:i386 libc6-dev:i386 gdb libsdl2-dev libglew-dev:i386 libfreetype6-dev:i386 -y 
```

Arch gcc6 & dependencies installation:
```bash
sudo pacman -U https://archive.archlinux.org/packages/g/gcc-multilib/gcc-multilib-6.3.1-2-x86_64.pkg.tar.xz 
https://archive.archlinux.org/packages/g/gcc-libs-multilib/gcc-libs-multilib-6.3.1-2-x86_64.pkg.tar.xz 
https://archive.archlinux.org/packages/l/lib32-gcc-libs/lib32-gcc-libs-6.3.1-2-x86_64.pkg.tar.xz && 
sudo cp -r /usr/include/c++/6.3.1/ /tmp/ && 
sudo pacman -S gdb gdb-common glew1.10 glew lib32-glew1.10 rsync lib62-gcc-libs gcc-libs-multilib gcc-multilib --noconfirm && yes |
sudo cp -r  /tmp/6.3.1/ /usr/include/c++/
```

If you don't use Ubuntu or Arch, here's the list of what cathook requires:

* `gcc-6`
* `g++-6`
* `gcc-6-multilib`
* `g++-6-multilib`
* `glew`
* `gdb` (for the injection script, you can use different injector if you want. Doesnt work with linux-inject!)
* `libc6-dev:i386`
* `libssl-dev:i386`(Below isnt needed yet)
* `libsdl2-dev`
* `libglew-dev:i386`
* `libfreetype6-dev:i386` 


Cathook installation script:
```bash
git clone https://github.com/oneechanhax/nekohook && cd nekohook && make
```

**Errors while installing?**

`/usr/include/c++/5/string:38:28: fatal error: bits/c++config.h: No such file or directory`
You don't have gcc-multilib-6 installed correctly.

If you are using another distro, make sure to have g++-6, gdb, libc6 and build essentials installed.

## Updating cathook
Run the `update` script in cathook folder.

## Injection
`sudo ./attach` to attach cathook into your game. Optionally, you can provide an argument number (0-n - #) to provide the game process ID (for bots).

`sudo ./attach-backtrace` to attach and print backtrace incase game crashes. Be sure to build a debug version. Some users report that this causes FPS drop in-game. This is recommended to grab a log of what went wrong if Cathook is crashing on you.
