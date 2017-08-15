#
# Super l33t code to downloade gcc to v6, update cathook, then re-upgrade to v7
# Script was created to automate the problem of downgrading to gcc6 to update cathook, then update to gcc7 to launch steam
# This should only be useful for arch users who can not install gcc6 and gcc7 as seperate packages due to either lack of knowledge or technical reasons
#

if [ $EUID == 0 ]; then
	echo "This script must be ran as root" # Standard root check to not break anything if anything is breakable.
	exit
fi

if [ -e "/var/cache/pacman/pkg/lib32-gcc-libs-6.3.1-2-x86_64.pkg.tar.xz" ] # Check if user even has gcc6
	then
		sudo pacman -U /var/cache/pacman/pkg/lib32-gcc-libs-6.3.1-2-x86_64.pkg.tar.xz /var/cache/pacman/pkg/gcc-libs-multilib-6.3.1-2-x86_64.pkg.tar.xz /var/cache/pacman/pkg/gcc-multilib-6.3.1-2-x86_64.pkg.tar.xz # Downgrade to gcc6
	else
		echo "Weird, it doesn't appear that you have gcc6 installed." # Exit script if gcc6 isnt found,
		echo "This would have normally been installed if you ran the" # which is impossible if they
		echo "arch dependencies script provided by the cathook page." # followed the installation
		echo "How about you go do that first before running cathook." # guide properly.
		exit
fi

cd ~/cathook # cd into cathook
sudo ./update # run cathook's update script
cd - # cd back into what we were in before the previous cd
sudo pacman -Syu # update the system (and gcc)
