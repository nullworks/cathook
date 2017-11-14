 
/*
 *
 *	The part of the framework to contain information about the game we are in.
 *
 *
 */

#include <string>
#include <vector>

class CGameInfo {
public:
	
	bool in_game = false;	// To identify if the game is ingame, please set this wherever you can
	std::vector<std::string> authors = {"Onee-chan", "nullifiedcat"}; // Add to this when you create your module to get credit
	std::string loaded_modules = "";	// Add your module to the string to notify whats loaded
	
};

extern CGameInfo g_GameInfo;