#pragma once
#include "settings/Bool.hpp"
#include "usercmd.hpp"
#include <map>
namespace criticals
{
extern settings::Boolean enabled;
extern settings::Boolean melee;
extern std::map<int, std::vector<int>> crit_cmds;
extern size_t current_index;
extern bool isEnabled();
extern bool force_crit_this_tick;
void fixBucket(IClientEntity *weapon, CUserCmd *cmd);
} // namespace criticals
