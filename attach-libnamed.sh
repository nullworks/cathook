#!/usr/bin/env bash

# Thank you LWSS
# https://github.com/LWSS/Fuzion/commit/a53b6c634cde0ed47b08dd587ba40a3806adf3fe

RUNUSER="sudo -u $(logname)"
$RUNUSER bash ./scripts/updater --autoupdater

line=$(pgrep -u $SUDO_USER hl2_linux)
arr=($line)
inst=$1
if [ $# == 0 ]; then
    inst=0
fi

if [ ${#arr[@]} == 0 ]; then
    echo TF2 isn\'t running!
    exit
fi

if [ $inst -gt ${#arr[@]} ] || [ $inst == ${#arr[@]} ]; then
    echo wrong index!
    exit
fi

proc=${arr[$inst]}

echo Running instances: "${arr[@]}"
echo Attaching to "$proc"

# pBypass for crash dumps being sent
# You may also want to consider using -nobreakpad in your launch options.
sudo mkdir -p /tmp/dumps # Make it as root if it doesnt exist
sudo chown root:root /tmp/dumps # Claim it as root
sudo chmod 000 /tmp/dumps # No permissions

# Get a Random name from the build_names file.
FILENAME=$(shuf -n 1 build_names)

# Create directory if it doesn't exist
if [ ! -d "/lib/i386-linux-gnu/" ]; then
    sudo mkdir /lib/i386-linux-gnu/
fi

# In case this file exists, get another one. ( checked it works )
while [ -f "/lib/i386-linux-gnu/${FILENAME}" ]; do
    FILENAME=$(shuf -n 1 build_names)
done

# echo $FILENAME > build_id # For detaching

sudo cp "bin/libcathook.so" "/lib/i386-linux-gnu/${FILENAME}"

echo loading "$FILENAME" to "$proc"

sudo gdb -n -q -batch \
     -ex "attach $proc" \
     -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
     -ex "call \$dlopen(\"/lib/i386-linux-gnu/$FILENAME\", 1)" \
     -ex "call dlerror()" \
     -ex 'print (char *) $2' \
     -ex "detach" \
     -ex "quit"

sudo rm "/lib/i386-linux-gnu/${FILENAME}"
