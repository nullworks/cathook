/*
 * CREDITS TO ONEE-CHAN FOR IMPROVED FOLLOWBOT
 *
 *
 *
 */

#include "common.hpp"
#include <hacks/FollowBot.hpp>
#include <settings/Bool.hpp>
#include "navparser.hpp"
#include "NavBot.hpp"
#include "soundcache.hpp"
#include "playerresource.h"
#include "PlayerTools.hpp"

namespace hacks::shared::followbot
{
static settings::Boolean enable{ "follow-bot.enable", "false" };
static settings::Boolean roambot{ "follow-bot.roaming", "true" };
static settings::Boolean follow_friends{ "follow-bot.prioritize-friends", "false" };
static settings::Boolean follow_party_leader{ "follow-bot.prioritize-leader", "false" };
static settings::Boolean draw_crumb{ "follow-bot.draw-crumbs", "false" };
static settings::Float follow_distance{ "follow-bot.distance", "175" };
static settings::Float additional_distance{ "follow-bot.ipc-distance", "100" };
static settings::Float follow_activation{ "follow-bot.max-range", "1000" };
static settings::Boolean mimic_slot{ "follow-bot.mimic-slot", "false" };
static settings::Boolean always_medigun{ "follow-bot.always-medigun", "false" };
static settings::Boolean sync_taunt{ "follow-bot.taunt-sync", "false" };
static settings::Boolean change{ "follow-bot.change-roaming-target", "false" };
static settings::Boolean autojump{ "follow-bot.jump-if-stuck", "true" };
static settings::Boolean afk{ "follow-bot.switch-afk", "true" };
static settings::Int afktime{ "follow-bot.afk-time", "15000" };
static settings::Boolean corneractivate{ "follow-bot.corners", "true" };
static settings::Int steam_var{ "follow-bot.steamid", "0" };
static settings::Boolean ignore_textmode{ "follow-bot.ignore-textmode", "true" };
static settings::Boolean mimic_crouch{ "follow-bot.mimic-crouch", "true" };
static settings::Boolean autozoom_if_idle{ "follow-bot.autozoom-if-idle", "true" };

namespace nb = hacks::tf2::NavBot;

static Timer navBotInterval{};
static unsigned steamid = 0x0;

static CatCommand follow_steam("fb_steam", "Follow Steam Id", [](const CCommand &args) {
    if (args.ArgC() < 2)
    {
        steam_var = 0;
        return;
    }
    try
    {
        steam_var = std::stoul(args.Arg(1));
        logging::Info("Stored Steamid: %u", steamid);
    }
    catch (std::invalid_argument)
    {
        logging::Info("Invalid Argument! resetting steamid.");
        steam_var = 0;
        return;
    }
});

static CatCommand steam_debug("debug_steamid", "Print steamids", []() {
    for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
    {
        auto ent = ENTITY(i);
        logging::Info("%u", ent->player_info.friendsID);
    }
});

// Something to store breadcrumbs created by followed players
static std::vector<Vector> breadcrumbs;
static constexpr int crumb_limit = 64; // limit

static bool followcart{ false };

// Followed entity, externed for highlight color
int follow_target  = 0;
static bool inited = false;

static Timer lastTaunt{}; // time since taunt was last executed, used to avoid kicks
static Timer lastJump{};
static Timer crouch_timer{};                          // Mimic crouch
static std::array<Timer, PLAYER_ARRAY_SIZE> afkTicks; // for how many ms the player hasn't been moving

bool isIdle()
{
    if (!enable || !autozoom_if_idle || !follow_target)
        return false;
    float follow_dist = *follow_distance;
#if ENABLE_IPC
    if (ipc::peer)
        follow_dist += *additional_distance * ipc::peer->client_id;
#endif

    auto target = ENTITY(follow_target);
    if (CE_BAD(target))
        return false;
    auto tar_orig        = target->m_vecOrigin();
    auto loc_orig        = LOCAL_E->m_vecOrigin();
    float dist_to_target = loc_orig.DistTo(tar_orig);

    // If we are zoomed, we should stop zooming a bit later to avoid zooming in again in a few CMs
    float multiplier = (g_pLocalPlayer->holding_sniper_rifle && g_pLocalPlayer->bZoomed) ? 1.05f : 0.95f;
    follow_dist      = std::clamp(follow_dist * 1.5f * multiplier, 200.0f * multiplier, std::max(500.0f * multiplier, follow_dist));

    return dist_to_target <= follow_dist;
}

static void checkAFK()
{
    for (int i = 1; i < g_GlobalVars->maxClients; i++)
    {
        if (soundcache::GetSoundLocation(i))
        {
            afkTicks[i].update();
        }
#if ENABLE_TEXTMODE
        auto entity = ENTITY(i);
        if (CE_BAD(entity))
            continue;
        if (!CE_VECTOR(entity, netvar.vVelocity).IsZero(60.0f))
        {
            afkTicks[i].update();
        }
#endif
    }
}

static void init()
{
    for (int i = 0; i < afkTicks.size(); i++)
    {
        afkTicks[i].update();
    }
    inited = true;
    return;
}

// auto add checked crumbs for the walkbot to follow
static void addCrumbs(CachedEntity *target, Vector corner = g_pLocalPlayer->v_Origin)
{
    breadcrumbs.clear();
    if (g_pLocalPlayer->v_Origin != corner)
    {
        Vector dist       = corner - g_pLocalPlayer->v_Origin;
        int maxiterations = floor(corner.DistTo(g_pLocalPlayer->v_Origin)) / 40;
        for (int i = 0; i < maxiterations; i++)
        {
            breadcrumbs.push_back(g_pLocalPlayer->v_Origin + dist / vectorMax(vectorAbs(dist)) * 40.0f * (i + 1));
        }
    }

    Vector dist       = target->m_vecOrigin() - corner;
    int maxiterations = floor(corner.DistTo(target->m_vecOrigin())) / 40;
    for (int i = 0; i < maxiterations; i++)
    {
        breadcrumbs.push_back(corner + dist / vectorMax(vectorAbs(dist)) * 40.0f * (i + 1));
    }
}

static void addCrumbPair(CachedEntity *player1, CachedEntity *player2, std::pair<Vector, Vector> corners)
{
    Vector corner1 = corners.first;
    Vector corner2 = corners.second;

    {
        Vector dist       = corner1 - player1->m_vecOrigin();
        int maxiterations = floor(corner1.DistTo(player1->m_vecOrigin())) / 40;
        for (int i = 0; i < maxiterations; i++)
        {
            breadcrumbs.push_back(player1->m_vecOrigin() + dist / vectorMax(vectorAbs(dist)) * 40.0f * (i + 1));
        }
    }
    {
        Vector dist       = corner2 - corner1;
        int maxiterations = floor(corner2.DistTo(corner1)) / 40;
        for (int i = 0; i < maxiterations; i++)
        {
            breadcrumbs.push_back(corner1 + dist / vectorMax(vectorAbs(dist)) * 40.0f * (i + 1));
        }
    }
    {
        Vector dist       = player2->m_vecOrigin() - corner2;
        int maxiterations = floor(corner2.DistTo(player2->m_vecOrigin())) / 40;
        for (int i = 0; i < maxiterations; i++)
        {
            breadcrumbs.push_back(corner2 + dist / vectorMax(vectorAbs(dist)) * 40.0f * (i + 1));
        }
    }
}
/* Order:
 *   No Class = 0,
 *   tf_scout = 1,
 *   tf_sniper = 2,
 *   tf_soldier = 3,
 *   tf_demoman = 4,
 *   tf_medic = 5,
 *   tf_heavy = 6,
 *   tf_pyro = 7,
 *   tf_spy = 8,
 *   tf_engineer = 9
 */

// Higher number = higher priority.
static constexpr int priority_list[10][10] = {
    /*0  1  2  3  4  5  6  7  8  9 */
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // No class
    { 0, 8, 2, 7, 6, 2, 5, 1, 0, 0 }, // Scout
    { 0, 1, 8, 2, 2, 2, 2, 1, 0, 0 }, // Sniper
    { 0, 6, 1, 8, 7, 5, 6, 4, 0, 0 }, // Soldier
    { 0, 6, 1, 7, 8, 5, 6, 4, 0, 0 }, // Demoman
    { 0, 3, 1, 7, 7, 1, 8, 2, 0, 0 }, // Medic
    { 0, 2, 1, 7, 6, 4, 8, 5, 0, 0 }, // Heavy
    { 0, 6, 1, 6, 5, 3, 5, 8, 0, 0 }, // Pyro
    { 0, 1, 1, 1, 1, 1, 1, 1, 8, 0 }, // Spy
    { 0, 1, 1, 1, 1, 1, 1, 1, 0, 8 }  // Engineer
};

int ClassPriority(CachedEntity *ent)
{
    int local_class = g_pPlayerResource->GetClass(LOCAL_E);
    int ents_class  = g_pPlayerResource->GetClass(ent);
    return priority_list[local_class][ents_class];
}

static int lastent = 0;
static Timer waittime{};
static Timer navinactivity{};
static bool navtarget = false;

static bool startFollow(CachedEntity *entity, bool useNavbot)
{
    if (CE_GOOD(entity) && !follow_activation || entity->m_flDistance() <= (float) follow_activation)
    {
        if (corneractivate)
        {
            Vector indirectOrigin = VischeckCorner(LOCAL_E, entity, *follow_activation / 2,
                                                   true); // get the corner location that the
            // future target is visible from
            std::pair<Vector, Vector> corners;
            if (!indirectOrigin.z && entity->m_IDX == lastent) // if we couldn't find it, run
            // wallcheck instead
            {
                corners = VischeckWall(LOCAL_E, entity, float(follow_activation) / 2, true);
                if (!corners.first.z || !corners.second.z)
                    return false;
                // addCrumbs(LOCAL_E, corners.first);
                // addCrumbs(entity, corners.second);
                addCrumbPair(LOCAL_E, entity, corners);
                navtarget = false;
                return true;
            }
            if (indirectOrigin.z)
            {
                navtarget = false;
                addCrumbs(entity, indirectOrigin);
                return true;
            }
        }
        else
        {
            if (VisCheckEntFromEnt(LOCAL_E, entity))
            {
                navtarget = false;
                return true;
            }
        }
    }
    if (useNavbot)
    {
        if (nav::navTo(entity->m_vecOrigin(), 8, true, false))
        {
            navtarget = true;
            return true;
        }
    }
    return false;
}

static bool isValidTarget(CachedEntity *entity)
{
    if (CE_INVALID(entity)) // Exist
        return false;
    // Check if already following
    if (entity->m_IDX == follow_target)
        return false;
    // Follow only players
    if (entity->m_Type() != ENTITY_PLAYER)
        return false;
    // Don't follow yourself
    if (entity == LOCAL_E)
        return false;
    // Don't follow dead players
    if (!g_pPlayerResource->isAlive(entity->m_IDX))
        return false;
    if (IsPlayerDisguised(entity) || IsPlayerInvisible(entity))
        return false;
    // Don't follow target that was determined afk
    if (afk && afkTicks[entity->m_IDX].check(*afktime))
        return false;
    if (ignore_textmode && playerlist::AccessData(entity).state == playerlist::k_EState::TEXTMODE)
        return false;
    return true;
}

static void cm()
{
    if (!enable || CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer() || CE_BAD(LOCAL_W))
    {
        follow_target = 0;
        if (nb::task::current_task == nb::task::followbot)
            nb::task::current_task = nb::task::none;
        return;
    }
    if (!inited)
        init();

    if (nb::task::current_task == nb::task::health || nb::task::current_task == nb::task::ammo)
    {
        follow_target = 0;
        return;
    }

    if (afk)
        checkAFK();

    // Still good check
    if (follow_target)
    {
        // Overflow protection
        if (breadcrumbs.size() > crumb_limit)
            follow_target = 0;
        // Still good check
        else if (CE_INVALID(ENTITY(follow_target)) || !ENTITY(follow_target)->m_vecDormantOrigin() || IsPlayerInvisible(ENTITY(follow_target)))
            follow_target = 0;
    }

    if (!follow_target)
    {
        breadcrumbs.clear(); // no target == no path
        crouch_timer.update();
    }

    bool isNavBotCM           = navBotInterval.test_and_set(3000) && nav::prepare();
    bool foundPreferredTarget = false;

    // Target Selection
    // TODO: Reduce code duplication
    if (steamid || follow_friends || follow_party_leader)
    {
        auto &valid_target = follow_target;
        // Find a target with the steam id, as it is prioritized
        auto ent_count = g_IEngine->GetMaxClients();
        if (steamid)
        {
            if (ENTITY(valid_target)->player_info.friendsID != steamid)
            {
                for (int i = 1; i <= ent_count; i++)
                {
                    auto entity = ENTITY(i);
                    if (!isValidTarget(entity))
                        continue;
                    // No enemy check, since steamid is very specific
                    if (steamid != entity->player_info.friendsID) // steamid check
                        continue;
                    if (startFollow(entity, isNavBotCM))
                    {
                        navinactivity.update();
                        follow_target        = entity->m_IDX;
                        foundPreferredTarget = true;
                        break;
                    }
                }
            }
            else
                foundPreferredTarget = true;
        }

        if (follow_party_leader && !foundPreferredTarget)
        {
            re::CTFPartyClient *pc = re::CTFPartyClient::GTFPartyClient();
            if (pc)
            {
                CSteamID steamid;
                pc->GetCurrentPartyLeader(steamid);
                auto accountid = steamid.GetAccountID();
                // if (steamid.GetAccountID() != ENTITY(follow_target)->player_info.friendsID)
                //    continue;

                if (accountid != ENTITY(valid_target)->player_info.friendsID)
                {
                    for (int i = 1; i <= ent_count; i++)
                    {
                        auto entity = ENTITY(i);
                        if (!isValidTarget(entity))
                            continue;
                        if (entity->m_bEnemy())
                            continue;
                        if (accountid != entity->player_info.friendsID)
                            continue;
                        if (startFollow(entity, isNavBotCM))
                        {
                            navinactivity.update();
                            follow_target        = entity->m_IDX;
                            foundPreferredTarget = true;
                            break;
                        }
                    }
                }
                else
                    foundPreferredTarget = true;
            }
        }

        if (follow_friends && !foundPreferredTarget)
        {
            if (!playerlist::IsFriend(ENTITY(valid_target)))
            {
                for (int i = 1; i <= ent_count; i++)
                {
                    auto entity = ENTITY(i);
                    if (!isValidTarget(entity))
                        continue;
                    if (entity->m_bEnemy())
                        continue;
                    if (!playerlist::IsFriend(entity))
                        continue;
                    if (startFollow(entity, isNavBotCM))
                    {
                        navinactivity.update();
                        follow_target        = entity->m_IDX;
                        foundPreferredTarget = true;
                        break;
                    }
                }
            }
            else
                foundPreferredTarget = true;
        }
    }

    if (follow_target)
        isNavBotCM = false;

    // If we dont have a follow target from that, we look again for someone
    // else who is suitable
    if (roambot && !foundPreferredTarget && (!follow_target || change || ClassPriority(ENTITY(follow_target)) < 6))
    {
        // Try to get a new target
        auto ent_count = followcart ? HIGHEST_ENTITY : g_IEngine->GetMaxClients();
        for (int i = 1; i <= ent_count; i++)
        {
            auto entity = ENTITY(i);
            if (!isValidTarget(entity))
                continue;
            if (!follow_target)
            {
                if (CE_INVALID(entity))
                    continue;
            }
            else
            {
                if (CE_BAD(entity))
                    continue;
            }
            if (entity->m_bEnemy())
                continue;
            // const model_t *model = ENTITY(follow_target)->InternalEntity()->GetModel();
            // FIXME follow cart/point
            /*if (followcart && model &&
                (lagexploit::pointarr[0] || lagexploit::pointarr[1] ||
                 lagexploit::pointarr[2] || lagexploit::pointarr[3] ||
                 lagexploit::pointarr[4]) &&
                (model == lagexploit::pointarr[0] ||
                 model == lagexploit::pointarr[1] ||
                 model == lagexploit::pointarr[2] ||
                 model == lagexploit::pointarr[3] ||
                 model == lagexploit::pointarr[4]))
                follow_target = entity->m_IDX;*/
            // favor closer entitys
            if (CE_GOOD(entity))
            {
                if (follow_target && ENTITY(follow_target)->m_flDistance() < entity->m_flDistance()) // favor closer entitys
                    continue;
                // check if new target has a higher priority than current
                // target
                if (ClassPriority(ENTITY(follow_target)) >= ClassPriority(ENTITY(i)))
                    continue;
            }
            if (startFollow(entity, isNavBotCM))
            {
                // ooooo, a target
                navinactivity.update();
                follow_target = i;
                afkTicks[i].update(); // set afk time to 03
                break;
            }
        }
    }

    lastent++;
    if (lastent > g_IEngine->GetMaxClients())
        lastent = 1;

    if (!follow_target)
    {
        crouch_timer.update();
        navtarget = false;
    }
    if (navtarget)
    {
        auto ent = ENTITY(follow_target);
        if (!nav::prepare())
        {
            follow_target = 0;
            navtarget     = 0;
        }

        if (!CE_GOOD(ent) || !startFollow(ent, false))
        {
            breadcrumbs.clear();
            static Timer navtimer{};
            if (CE_VALID(ent))
            {
                auto pos = ent->m_vecDormantOrigin();
                if (!g_pPlayerResource->isAlive(ent->m_IDX))
                {
                    follow_target = 0;
                }
                if (pos && navtimer.test_and_set(800))
                {
                    if (nav::navTo(*pos, 8, true, false))
                        navinactivity.update();
                }
            }
            if (navinactivity.check(5000))
            {
                follow_target = 0;
            }
            nb::task::current_task = nb::task::followbot;
            return;
        }
    }

    // last check for entity before we continue
    if (!follow_target)
    {
        if (nb::task::current_task == nb::task::followbot)
            nb::task::current_task = nb::task::none;
        return;
    }

    nb::task::current_task = nb::task::followbot;
    nav::clearInstructions();

    CachedEntity *followtar = ENTITY(follow_target);
    // wtf is this needed
    if (CE_BAD(followtar) || !followtar->m_bAlivePlayer())
    {
        follow_target = 0;
        return;
    }
    // Check if we are following a disguised/spy
    if (IsPlayerDisguised(followtar) || IsPlayerInvisible(followtar))
    {
        follow_target = 0;
        return;
    }
    // Crouch logic
    auto flags = CE_INT(followtar, netvar.iFlags);
    if (!(flags & FL_DUCKING) || !(flags & FL_ONGROUND))
        crouch_timer.update();
    else if (crouch_timer.check(500) && *mimic_crouch)
        current_user_cmd->buttons |= IN_DUCK;
    // check if target is afk
    if (afk)
    {
        if (afkTicks[follow_target].check(int(afktime)))
        {
            follow_target = 0;
            return;
        }
    }

    // Update timer on new target
    static Timer idle_time{};
    if (breadcrumbs.empty())
        idle_time.update();

    auto tar_orig       = followtar->m_vecOrigin();
    auto loc_orig       = LOCAL_E->m_vecOrigin();
    auto dist_to_target = loc_orig.DistTo(tar_orig);

    // If the player is close enough, we dont need to follow the path
    if ((dist_to_target < (float) follow_distance) && VisCheckEntFromEnt(LOCAL_E, followtar))
    {
        idle_time.update();
    }

    // Prune old and close crumbs that we wont need anymore, update idle
    // timer too
    for (int i = 0; i < breadcrumbs.size(); i++)
    {
        if (loc_orig.DistTo(breadcrumbs.at(i)) < 60.f)
        {
            idle_time.update();
            for (int j = 0; j <= i; j++)
                breadcrumbs.erase(breadcrumbs.begin());
        }
    }

    // New crumbs, we add one if its empty so we have something to follow
    if (breadcrumbs.empty() || (tar_orig.DistTo(breadcrumbs.at(breadcrumbs.size() - 1)) > 40.0F && DistanceToGround(ENTITY(follow_target)) < 45))
        breadcrumbs.push_back(tar_orig);

    // Tauntsync
    if (sync_taunt && HasCondition<TFCond_Taunting>(followtar) && lastTaunt.test_and_set(1000))
    {
        g_IEngine->ClientCmd("taunt");
    }

    // Follow the crumbs when too far away, or just starting to follow
#if ENABLE_IPC
    float follow_dist = *follow_distance;
    if (ipc::peer)
        follow_dist += *additional_distance * ipc::peer->client_id;
    if (dist_to_target > follow_dist)
#else
    if (dist_to_target > *follow_distance)
#endif
    {
        // Check for jump
        if (autojump && lastJump.check(1000) && (idle_time.check(2000) || DistanceToGround({ breadcrumbs[0].x, breadcrumbs[0].y, breadcrumbs[0].z + 5 }) > 47))
        {
            current_user_cmd->buttons |= IN_JUMP;
            lastJump.update();
        }
        // Check if still moving. 70 HU = Sniper Zoomed Speed
        if (idle_time.check(3000) && CE_VECTOR(g_pLocalPlayer->entity, netvar.vVelocity).IsZero(60.0f))
        {
            follow_target = 0;
            return;
        }
        // Basic idle check
        if (idle_time.test_and_set(5000))
        {
            follow_target = 0;
            return;
        }

        static float last_slot_check = 0.0f;
        if (g_GlobalVars->curtime < last_slot_check)
            last_slot_check = 0.0f;
        if (follow_target && ((always_medigun && g_pPlayerResource->GetClass(LOCAL_E) == tf_medic) || mimic_slot) && (g_GlobalVars->curtime - last_slot_check > 1.0f) && !g_pLocalPlayer->life_state && !CE_BYTE(ENTITY(follow_target), netvar.iLifeState))
        {

            // We are checking our slot so reset the timer
            last_slot_check = g_GlobalVars->curtime;

            // Get the follow targets active weapon
            int owner_weapon_eid        = (CE_INT(ENTITY(follow_target), netvar.hActiveWeapon) & 0xFFF);
            IClientEntity *owner_weapon = g_IEntityList->GetClientEntity(owner_weapon_eid);

            // If both the follow targets and the local players weapons are
            // not null or dormant
            if (owner_weapon && CE_GOOD(g_pLocalPlayer->weapon()))
            {

                // IsBaseCombatWeapon()
                if (re::C_BaseCombatWeapon::IsBaseCombatWeapon(RAW_ENT(g_pLocalPlayer->weapon())) && re::C_BaseCombatWeapon::IsBaseCombatWeapon(owner_weapon))
                {

                    // Get the players slot numbers and store in some vars
                    int my_slot    = re::C_BaseCombatWeapon::GetSlot(RAW_ENT(g_pLocalPlayer->weapon()));
                    int owner_slot = re::C_BaseCombatWeapon::GetSlot(owner_weapon);

                    // If the local player is a medic and user settings
                    // allow, then keep the medigun out
                    if (g_pLocalPlayer->clazz == tf_medic && always_medigun)
                    {
                        if (my_slot != 1)
                        {
                            g_IEngine->ExecuteClientCmd("slot2");
                        }

                        // Else we attemt to keep our weapon mimiced with
                        // our follow target
                    }
                    else
                    {
                        if (my_slot != owner_slot)
                        {
                            g_IEngine->ExecuteClientCmd(format("slot", owner_slot + 1).c_str());
                        }
                    }
                }
            }
        }
        WalkTo(breadcrumbs[0]);
    }
    else
        idle_time.update();
} // namespace hacks::shared::followbot

#if ENABLE_VISUALS
static void draw()
{
    if (!enable || !draw_crumb)
        return;
    if (breadcrumbs.size() < 2)
        return;
    for (size_t i = 0; i < breadcrumbs.size() - 1; i++)
    {
        Vector wts1, wts2;
        if (draw::WorldToScreen(breadcrumbs[i], wts1) && draw::WorldToScreen(breadcrumbs[i + 1], wts2))
        {
            draw::Line(wts1.x, wts1.y, wts2.x - wts1.x, wts2.y - wts1.y, colors::white, 0.1f);
        }
    }
    Vector wts;
    if (!draw::WorldToScreen(breadcrumbs[0], wts))
        return;
    draw::Rectangle(wts.x - 4, wts.y - 4, 8, 8, colors::white);
    draw::RectangleOutlined(wts.x - 4, wts.y - 4, 7, 7, colors::white, 1.0f);
}
#endif

bool isEnabled()
{
    return *enable;
}

#if ENABLE_IPC
static CatCommand follow_me("fb_follow_me", "IPC connected bots will follow you", []() {
    if (!ipc::peer)
    {
        logging::Info("IPC isnt connected");
        return;
    }
    auto local_ent = LOCAL_E;
    if (!local_ent)
    {
        logging::Info("Cant get a local player");
        return;
    }
    player_info_s info;
    g_IEngine->GetPlayerInfo(local_ent->m_IDX, &info);
    auto steam_id = info.friendsID;
    if (!steam_id)
    {
        logging::Info("Cant get steam-id, the game module probably doesnt "
                      "support it.");
        return;
    }
    // Construct the command
    std::string tmp = CON_PREFIX + follow_steam.name + " " + std::to_string(steam_id);
    if (tmp.length() >= 63)
    {
        ipc::peer->SendMessage(0, 0, ipc::commands::execute_client_cmd_long, tmp.c_str(), tmp.length() + 1);
    }
    else
    {
        ipc::peer->SendMessage(tmp.c_str(), 0, ipc::commands::execute_client_cmd, 0, 0);
    }
});
#endif
void rvarCallback(settings::VariableBase<int> &var, int after)
{
    if (after < 0)
        return;
    steamid = after;
}

static InitRoutine runinit([]() {
    EC::Register(EC::CreateMove, cm, "cm_followbot", EC::average);
#if ENABLE_VISUALS
    EC::Register(EC::Draw, draw, "draw_followbot", EC::average);
#endif
    steam_var.installChangeCallback(rvarCallback);
});
} // namespace hacks::shared::followbot
