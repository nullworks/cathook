#pragma once
#include <string>
#include "settings/String.hpp"

class CSteamID;

extern settings::String force_name;
extern std::string name_forced;
std::string GetNamestealName(CSteamID steam_id);
