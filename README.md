
# Nekohook Modular Fork
![banner](https://cdn.discordapp.com/attachments/371237920545439745/403928612887068692/nekohook-banner.png)

## Discord Server
[NekoHook Discord Server](https://discord.gg/Jp8SKtG)
Link is down to prevent raiding/spamming, give it a bit.
If catbot is down, just wait until it comes online to register. Make sure to read the rules!

## About
Nekohook is an extremely modular training software framework designed to work with many games, even games not using the source engine!   

This nekohook fork is currently a work in progress but you may request a personal build at the discord server if you've met the criteria listed there.   

Nekohook is currently VAC undetected!

# Developer info
Windows support is in its infancy, if you would like to support windows development, join the discord server and ask how you can help. I only need developers though as its alot of small errors that need to be fixed in the project.   

There isnt documentation *yet* so I'll give you a quick run down.   
For framework,   
You can use GetEntity to get a entity handle, then pass it to any function in src/framework/entitys.hpp to get info.   
This is somewhat the same for other parts of the framework, but dont ever directly include a module into the framework.   
please keep the format in the framework the same as the rest of the code and comment heavily. Heavily commenting allows anyone looking to use the framework have an easy time reading and understanding how it works, great for education purposes. If you cant use the format, try to keep to googles standard at least, or clang-format your own code.   
For modules,   
On init, you should be setting the frameworks functions to be your own.   
The functions recieve entity handles which you can use to know what the framework is trying to refrence.   
From there, you can provide the framework with the information it wants, allowing the framework to function.   
The cmake is on a todo to re-write, it will auto generate headers to allow the module manage to load your module.
The framework is going through big changes almost every day, I can try to keep the changes to a minimum but sometimes that wont always be possible and modules may break.   

# Installation and usage
Nekohook uses game modules, which aren't included by default.   
If you would like to develop one, please continue.   
```bash
git clone --recursive https://github.com/oneechanhax/nekohook && cd nekohook && ./build
```

## Updating
Updates come from the nekohook server, you may pull those yourself and place them into the nekohook directory to update.   
If you are developing, pull from github like normal.
```bash
git pull
```

## Injecting
The injector is quite different since cathook. Use the attach script like so.   
You may find more useful development tools with -h, and maybe even a few easter eggs ;)
```bash
sudo ./attach
```

## Issues
You may report issues to the github, or through discord.    
Please explain what the issue is and how to recreate it.   
If you are developing, providing a backtrace or even a pull request is recommended.
