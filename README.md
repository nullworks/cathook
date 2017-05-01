# Discord Server
[Official Discord Server](https://discord.gg/7bu3AFw)

# INSTALLATION

Ubuntu (and probably Debian) users can run this script:

```
sudo apt update && sudo apt install build-essential software-properties-common -y && sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y && sudo apt update && sudo apt install gcc-snapshot -y && sudo apt update && sudo apt install libc6-dev gcc-6 g++-6 g++-6-multilib gdb -y && git clone --recursive https://github.com/nullifiedcat/cathook -b performance --single-branch && cd cathook && make -j4
```
Arch users will need git and gcc-multilib

```
git clone --recursive https://github.com/nullifiedcat/cathook -b performance --single-branch ; cd cathook ; make -j4 
```
**Errors while installing?** 

`/usr/include/c++/5/string:38:28: fatal error: bits/c++config.h: No such file or directory` - You don't have g++6 or g++6 multilib installed correctly

`src/<any file>: fatal error: mathlib/vector.h: No such file or directory` - You didn't download Source SDK. **DO NOT DOWNLOAD CATHOOK USING "DOWNLOAD .ZIP" FROM GITHUB. USE git clone --recursive**!

If you are using other distro, make sure to have g++-6, gdb, libc6 and build essentials installed.

## Updating cathook
Run the `update` script in cathook folder.

## Injection
`sudo ./attach` to attach to tf2 process (can take argument number 0-N - # of tf2 instance to attach to (for bots))

`sudo ./attach-backtrace` to attach and print backtrace if tf2 crashes. Some users reported that this method makes you get less FPS ingame.

## Followbots

Followbot installation is quite complex and I won't cover it fully here.
You have to have several user accounts ready to run tf2 - use google for that.
You can ask someone in my discord server for help with installation.
To control followbots, you need to download and install `cathook-ipc-server`.

### Followbot server installation script
```
git clone --recursive https://github.com/nullifiedcat/cathook-ipc-server && \
cd cathook-ipc-server && \
make -j4
```
### Updating script is the same as updating cathook

### Running followbot server
`./bin/cathook-ipc-server` or `./bin/cathook-ipc-server &>/dev/null &` to run it in background
