
/*
 *
 *	The part of the framework to interact with the game we are in.
 *
 *
 */

#include <string>
#include <vector>

#include "../util/functional.hpp"

namespace game {

// To identify if the game is ingame
extern CMFunction<bool()> GetInGame;

// Add to this when you create your module to get credit
extern std::vector<std::string> authors;

// Add your module to the string to notify whats loaded
// TODO, make not unused
extern std::vector<std::string> loaded_modules;

}
