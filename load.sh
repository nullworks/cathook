#!/usr/bin/env bash

prefix="\e[34m\e[1mSpookware >\e[0m"
error_prefix="\e[91m\e[1mSpookware >\e[0m"
success_prefix="\e[32m\e[1mSpookware >\e[0m"

# ----------------------------------------------- #
#                                                 #
#    (                                            #
#    )\ )                      )                  #
#    (()/(           )  (    ( /(    )            #
#    /(_))`  )   ( /(  )(   )\())( /(   (         #
#    (_))  /(/(   )(_))(()\ (_))/ )(_))  )\ )     #
#    / __|((_)_\ ((_)_  ((_)| |_ ((_)_  _(_/(     #
#    \__ \| '_ \)/ _` || '_||  _|/ _` || ' \))    #
#    |___/| .__/ \__,_||_|   \__|\__,_||_||_|     #
#         |_|                                     #
#                                                 #
# ----------------------------------------------- #

csgo=$(pidof csgo_linux64)

if [ -z "$csgo" ]; then
    echo -e "$error_prefix Please open Counter-Strike: Global Offensive."
    exit -1
fi

# Credit to aixxe from AimTuxOfficial
if grep -q "libnekohook.so" /proc/$csgo/maps; then
    echo -e "$error_prefix Spookware has already been injected."
    exit -1
fi

if [ -d ".git" ]; then
    echo -e "$prefix Detected development version of Spookware."
fi

# Allows only root to use ptrace. This is temporary until the user reboots the machine.
ptrace_input=$(sudo echo "2" | sudo tee /proc/sys/kernel/yama/ptrace_scope)

ptrace_last_line="${ptrace_input##*$'\n'}"

if [ "$ptrace_last_line" != "2" ]; then
    echo -e "$error_prefix Failed to set ptrace scope to root only. This \e[4mmay\e[24m be unsafe."
    while true; do
        echo -e -n "$error_prefix "
        read -p $'Do you wish to continue? (y/N) ' yn
        case $yn in
            [Yy]* ) break;;
            [Nn]* ) exit -1;;
            * ) echo -e "$error_prefix Please answer yes or no.";;
        esac
    done
fi

# Prevent crash dumps from being sent to kisak
sudo rm -rf /tmp/dumps
sudo mkdir /tmp/dumps
sudo chmod 000 /tmp/dumps

# Credit to LWSS for removing the "warning: Could not load shared library symbols for bin/linux64/launcher_client.so."
input="$(
sudo gdb -n -q -batch-silent \
  -ex "set logging on" \
  -ex "set logging file /dev/null" \
  -ex "attach $csgo" \
  -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
  -ex "call \$dlopen(\"/$(pwd)/libnekohook.so\", 1)" \
  -ex "detach" \
  -ex "quit"
)"

last_line="${input##*$'\n'}"

if [ "$last_line" != *"0x0"* ]; then
    echo -e "$success_prefix Spookware has been successfully injected."
else
    echo -e "$error_prefix Spookware has failed to inject. Please restart CS:GO and retry."
fi

if [ -f "$(pwd)/gdb.txt" ]; then
    sudo rm -rf gdb.txt
fi
