# Cathook Multihack
![banner](http://i.imgur.com/GkBmJFT.png)

cathook is a multihack for Team Fortress 2 for Linux. cathook includes some joke features like

* Always/Never spycrab
* Ignore Hoovy
* 100% Casual/Comp coin flip hack
* Encrypted chat
* Emoji ESP
* Fidget Spinner crosshair

and a lot of useful features, including

* Anti Backstab with option to say "No" voice command when spy tries to backstab you
* Heal Arrows hack (overheal an enemy for 12000 health with single huntsman arrow, you can also do it with buildings!)
* Extremely customizable spam (you can make spam lines that'll include name of random dead enemy pyro or sniper)
* Follow Bots
* Working crit hack


# Discord Server
[Official Discord Server](https://discord.gg/kvNVNSX)

# INSTALLATION

Ubuntu dependencies installation:
```bash
sudo apt update && sudo apt install build-essential software-properties-common -y && sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y && sudo apt update && sudo apt install gcc-snapshot -y && sudo apt update && sudo apt install git libc6-dev gcc-6 g++-6 libc6-dev:i386 g++-6-multilib gdb libsdl2-dev libglew-dev libfreetype6-dev libfreetype6-dev:i386 -y && 
```


Arch dependencies installation::
```bash
sudo pacman -Syu && sudo pacman -S gdb gdb-common glew1.10 lib32-glew1.10 --noconfirm && yes | sudo pacman -U https://archive.archlinux.org/packages/g/gcc-multilib/gcc-multilib-6.3.1-2-x86_64.pkg.tar.xz https://archive.archlinux.org/packages/g/gcc-libs-multilib/gcc-libs-multilib-6.3.1-2-x86_64.pkg.tar.xz https://archive.archlinux.org/packages/l/lib32-gcc-libs/lib32-gcc-libs-6.3.1-2-x86_64.pkg.tar.xz
```


Cathook installation script:
```bash
git clone --recursive https://github.com/nullifiedcat/cathook && cd cathook && make -j4 && bash update
```

**Errors while installing?**

`/usr/include/c++/5/string:38:28: fatal error: bits/c++config.h: No such file or directory`
You don't have gcc-multilib-6 installed correctly.

`src/<any file>: fatal error: mathlib/vector.h: No such file or directory`
You didn't download Source SDK. **DO NOT DOWNLOAD CATHOOK USING "DOWNLOAD .ZIP" FROM GITHUB. USE git clone --recursive!**

If you are using another distro, make sure to have g++-6, gdb, libc6 and build essentials installed.

## Updating cathook
Run the `update` script in cathook folder.

Cathook requires the shader folder provided in order to launch properly. To install this, run the `update-data` script. This will create a sym-link for the required folders.  
If you wish to do this manually, copy the tf-settings folder inside Cathook into your Team Fortress 2 folder and rename it to "cathook".  
You will want to update these files if you wish to have an updated menu.

## Injection
`sudo ./attach` to attach cathook into TF2. Optionally, you can provide an argument number (0-n - #) to provide the TF2 process ID (for bots).

`sudo ./attach-backtrace` to attach and print backtrace incase TF2 crashes. Some users report that this causes FPS drop in-game. This is recommended to grab a log of what went wrong if Cathook is crashing on you.

## Followbots
`cathook-ipc-server` allows you to run and control Followbots to do your evil bidding in-game. The installation for Followbots is quite complex, and will not be covered on this page. Obviously, you must have several user accounts ready to run TF2.  
A guide for Followbots can be found here: [How to setup and use followbots.](https://www.youtube.com/watch?v=kns5-nw7xUg)  
You may also ask someone in our discord server to help you out.

The installation script is as followed:
```bash
git clone --recursive https://github.com/nullifiedcat/cathook-ipc-server && cd cathook-ipc-server && make -j4
```
To run the Followbot server, run `./bin/cathook-ipc-server`. You can also use `./bin/cathook-ipc-server &>/dev/null &` to run it in background.
