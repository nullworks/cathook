/*
  Created on 23.06.18.
*/

#include <core/cvwrapper.hpp>
#include <unordered_map>
#include <hoovy.hpp>
#include <playerlist.hpp>
#include <online/Online.hpp>
#include <settings/Bool.hpp>
#include "PlayerTools.hpp"
#include "entitycache.hpp"

static settings::Int betrayal_limit{ "player-tools.betrayal-limit", "true" };

static settings::Bool taunting{ "player-tools.ignore.taunting", "true" };
static settings::Bool hoovy{ "player-tools.ignore.hoovy", "true" };

static settings::Bool online_notarget{ "player-tools.ignore.online.notarget",
                                       "true" };
static settings::Bool online_friendly_software{
    "player-tools.ignore.online.friendly-software", "true"
};
static settings::Bool online_only_verified{
    "player-tools.ignore.online.only-verified-accounts", "true"
};
static settings::Bool online_anonymous{ "player-tools.ignore.online.anonymous",
                                        "true" };

static std::unordered_map<unsigned, unsigned> betrayal_list{};

static CatCommand forgive_all("pt_forgive_all", "Clear betrayal list",
                              []() { betrayal_list.clear(); });

namespace player_tools
{

IgnoreReason shouldTargetSteamId(unsigned id)
{
    if (id == 0)
        return IgnoreReason::DO_NOT_IGNORE;

    if (betrayal_limit)
    {
        if (betrayal_list[id] > int(betrayal_limit))
            return IgnoreReason::DO_NOT_IGNORE;
    }

    auto &pl = playerlist::AccessData(id);
    if (playerlist::IsFriendly(pl.state))
        return IgnoreReason::LOCAL_PLAYER_LIST;

    auto *co = online::getUserData(id);
    if (co)
    {
        bool check_verified  = !online_only_verified || co->is_steamid_verified;
        bool check_anonymous = online_anonymous || !co->is_anonymous;

        if (check_verified && check_anonymous)
        {
            if (online_notarget && co->no_target)
                return IgnoreReason::ONLINE_NO_TARGET;
            if (online_friendly_software && co->is_using_friendly_software)
                return IgnoreReason::ONLINE_FRIENDLY_SOFTWARE;
        }
        // Always check developer status, no exceptions
        if (co->is_developer)
            return IgnoreReason::DEVELOPER;
    }

    return IgnoreReason::DO_NOT_IGNORE;
}
IgnoreReason shouldTarget(CachedEntity *entity)
{
    if (entity->m_Type() == ENTITY_PLAYER)
    {
        if (hoovy && IsHoovy(entity))
            return IgnoreReason::IS_HOOVY;
        if (taunting && HasCondition<TFCond_Taunting>(entity))
            return IgnoreReason::IS_TAUNTING;

        return shouldTargetSteamId(entity->player_info.friendsID);
    }

    return IgnoreReason::DO_NOT_IGNORE;
}

bool shouldAlwaysRenderEspSteamId(unsigned id)
{
    if (id == 0)
        return false;

    auto &pl = playerlist::AccessData(id);
    if (pl.state != playerlist::k_EState::DEFAULT)
        return true;

    auto *co = online::getUserData(id);
    if (co)
        return true;

    return false;
}
bool shouldAlwaysRenderEsp(CachedEntity *entity)
{
    if (entity->m_Type() == ENTITY_PLAYER)
    {
        return shouldAlwaysRenderEspSteamId(entity->player_info.friendsID);
    }

    return false;
}

#if ENABLE_VISUALS
std::optional<colors::rgba_t> forceEspColorSteamId(unsigned id)
{
    if (id == 0)
        return std::nullopt;

    auto pl = playerlist::Color(id);
    if (pl != colors::empty)
        return std::optional<colors::rgba_t>{ pl };

    auto *co = online::getUserData(id);
    if (co)
    {
        if (co->has_color)
            return std::optional<colors::rgba_t>{ co->color };
        if (co->rainbow)
            return std::optional<colors::rgba_t>{ colors::RainbowCurrent() };
    }

    return std::nullopt;
}
std::optional<colors::rgba_t> forceEspColor(CachedEntity *entity)
{
    if (entity->m_Type() == ENTITY_PLAYER)
    {
        return forceEspColorSteamId(entity->player_info.friendsID);
    }

    return std::nullopt;
}
#endif

void onKilledBy(unsigned id)
{
    auto reason = shouldTargetSteamId(id);
    if (reason != IgnoreReason::DO_NOT_IGNORE)
    {
        // We ignored the gamer, but they still shot us
        if (betrayal_list.find(id) == betrayal_list.end())
            betrayal_list[id] = 0;
        betrayal_list[id]++;
    }
}

void onKilledBy(CachedEntity *entity)
{
    onKilledBy(entity->player_info.friendsID);
}
} // namespace player_tools