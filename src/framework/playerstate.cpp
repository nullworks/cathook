
/*
 *
 * Gotta have an easy way to manage playerstates
 *
 */

#include "../util/functions.hpp"
#include "../util/logging.hpp"
#include "entitys.hpp"
#include "console.hpp"

// Playerstates for steamid and name respectivly
CatFastMap<int, int> PlayerStates_steam;
std::unordered_map<std::string, int> PlayerStates_name;

enum{
  PSTATE_FRIENDLY = -1,
  PSTATE_NONE = 0,
  PSTATE_RAGE = 1
};

int GetPlayerState(CatEntity* entity){
  // If we can get steam ids, we will use that to deal with playerstates
  auto id = GetSteamId(entity);
  if (id) {
    // If we find a playerstate, we return it
    auto find = PlayerStates_steam.find(id);
    if (find != PlayerStates_steam.end())
      return find->second;
  // Without steamids, we can try names instead, more expensive but still functional
  } else {
    auto find = PlayerStates_name.find(GetName(entity));
    if (find != PlayerStates_name.end())
      return find->second;
  }
  return PSTATE_NONE;
}

void SetPlayerState(CatEntity* entity, int state) {
  // If we can get steam ids, we will use that to deal with playerstates
  auto id = GetSteamId(entity);
  if (id) {
    // If we find a playerstate, we return it
    PlayerStates_steam.set({id, state});
    return;
  // Without steamids, we can try names instead, more expensive but still functional
  } else {
    auto find = PlayerStates_name.find(GetName(entity));
    if (find != PlayerStates_name.end())
      find->second = state;
    return;
  }
  g_CatLogging.log("Cant set playerstate!");
}
