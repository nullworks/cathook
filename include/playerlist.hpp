/*
 * playerlist.hpp
 *
 *  Created on: Apr 11, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"

namespace playerlist
{

constexpr int SERIALIZE_VERSION = 3;

enum class k_EState
{
    STATE_FIRST = 0,
    DEFAULT     = STATE_FIRST,
    FRIEND,
    RAGE,
    IPC,
    TEXTMODE,
    CAT,
    PARTY,
    STATE_LAST = PARTY
};
#if ENABLE_VISUALS
extern std::array<rgba_t, 7> k_Colors;
static_assert(sizeof(rgba_t) == sizeof(float) * 4, "player list is going to be incompatible with no visual version");
#endif
extern const std::string k_Names[];
extern const char *const k_pszNames[];
extern const std::array<std::pair<k_EState, size_t>, 5> k_arrGUIStates;

struct userdata
{
    k_EState state{ k_EState::DEFAULT };
#if ENABLE_VISUALS
    rgba_t color{ 0, 0, 0, 0 };
#else
    char color[16]{};
#endif
    float inventory_value{ 0 };
    unsigned deaths_to{ 0 };
    unsigned kills{ 0 };
};

extern boost::unordered_flat_map<unsigned, userdata> data;

void Save();
void Load();

constexpr bool IsFriendly(k_EState state)
{
    return state != k_EState::RAGE && state != k_EState::DEFAULT && state != k_EState::CAT;
}
#if ENABLE_VISUALS
rgba_t Color(unsigned steamid);
rgba_t Color(CachedEntity *player);
#endif
userdata &AccessData(unsigned steamid);
userdata &AccessData(CachedEntity *player);
bool IsDefault(unsigned steamid);
bool IsDefault(CachedEntity *player);
bool IsFriend(unsigned steamid);
bool IsFriend(CachedEntity *player);
bool ChangeState(unsigned int steamid, k_EState state, bool force = false);
bool ChangeState(CachedEntity *entity, k_EState state, bool force = false);
} // namespace playerlist
