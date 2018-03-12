
/*
 *
 *	The part of the framework to contain information about the game we are in.
 *
 *
 */

#include "game.hpp"

namespace gameinfo {

// To identify if the game is ingame
CMFunction<bool()> IsInGame {[](){return true;}};

// Add to this when you create your module to get credit
std::vector<std::string> authors = {"Onee-chan", "nullifiedcat", "F1ssi0N"};

// Add your module to the string to notify whats loaded
// TODO, make not unused
std::vector<std::string> loaded_modules;

}
