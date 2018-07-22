
/*
 * CREDITS TO ONEE-CHAN FOR IMPROVED FOLLOWBOT
 *
 *
 *
 */

#include "common.hpp"
#include <hacks/FollowBot.hpp>
#include <hacks/LagExploit.hpp>
#include <glez/draw.hpp>

namespace hacks::shared::followbot
{

CatVar followbot(CV_SWITCH, "fb", "0", "Followbot Switch",
                 "Set to 1 in followbots' configs");
bool followcart = false;
CatVar roambot(CV_SWITCH, "fb_roaming", "1", "Roambot",
               "Followbot will roam free, finding targets it can");
static CatVar draw_crumb(CV_SWITCH, "fb_draw", "1", "Draw crumbs",
                         "Self explanitory");
static CatVar follow_distance(CV_INT, "fb_distance", "175", "Follow Distance",
                              "How close the bots should stay to the target");
static CatVar follow_activation(CV_INT, "fb_activation", "1000",
                                "Activation Distance",
                                "How close a player should be until the "
                                "followbot will pick them as a target");
unsigned steamid = 0x0;
CatCommand follow_steam("fb_steam", "Follow Steam Id",
                        [](const CCommand &args) {
                            if (args.ArgC() < 1)
                            {
                                steamid = 0x0;
                                return;
                            }
                            steamid = atol(args.Arg(1));

                        });
static CatVar mimic_slot(CV_SWITCH, "fb_mimic_slot", "0", "Mimic weapon slot",
                         "Mimic follow target's weapon slot");
static CatVar always_medigun(CV_SWITCH, "fb_always_medigun", "0",
                             "Always Medigun", "Always use medigun");
static CatVar sync_taunt(CV_SWITCH, "fb_sync_taunt", "0", "Synced taunt",
                         "Taunt when follow target does");
static CatVar change(CV_SWITCH, "fb_switch", "0", "Change followbot target",
                     "Always change roaming target when possible");
static CatVar autojump(CV_SWITCH, "fb_autojump", "1", "Autojump",
                       "Automatically jump if stuck");
static CatVar afk(CV_SWITCH, "fb_afk", "1", "Switch target if AFK",
                  "Automatically switch target if the target is afk");
static CatVar afktime(
    CV_INT, "fb_afk_time", "15000", "Max AFK Time",
    "Max time in ms spent standing still before player gets declared afk");
static CatVar corneractivate(
    CV_SWITCH, "fb_activation_corners", "1", "Activate around corners",
    "Try to find an activation path to an entity behind a corner.");

// Something to store breadcrumbs created by followed players
static std::vector<Vector> breadcrumbs;
static const int crumb_limit = 64; // limit

// Followed entity, externed for highlight color
int follow_target = 0;
static bool inited;

Timer lastTaunt{}; // time since taunt was last executed, used to avoid kicks
Timer lastJump{};
std::array<Timer, 32> afkTicks; // for how many ms the player hasn't been moving

void checkAFK()
{
    for (int i = 0; i < g_GlobalVars->maxClients; i++)
    {
        auto entity = ENTITY(i);
        if (CE_BAD(entity))
            continue;
        if (!CE_VECTOR(entity, netvar.vVelocity).IsZero(60.0f))
        {
            afkTicks[i].update();
        }
    }
}

void init()
{
    for (int i = 0; i < afkTicks.size(); i++)
    {
        afkTicks[i].update();
    }
    inited = true;
    return;
}

// auto add checked crumbs for the walkbot to follow
void addCrumbs(CachedEntity *target, Vector corner = g_pLocalPlayer->v_Origin)
{
    breadcrumbs.clear();
    if (g_pLocalPlayer->v_Origin != corner)
    {
        Vector dist       = corner - g_pLocalPlayer->v_Origin;
        int maxiterations = floor(corner.DistTo(g_pLocalPlayer->v_Origin)) / 40;
        for (int i = 0; i < maxiterations; i++)
        {
            breadcrumbs.push_back(g_pLocalPlayer->v_Origin +
                                  dist / vectorMax(vectorAbs(dist)) * 40.0f *
                                      (i + 1));
        }
    }

    Vector dist       = target->m_vecOrigin() - corner;
    int maxiterations = floor(corner.DistTo(target->m_vecOrigin())) / 40;
    for (int i = 0; i < maxiterations; i++)
    {
        breadcrumbs.push_back(
            corner + dist / vectorMax(vectorAbs(dist)) * 40.0f * (i + 1));
    }
}

void addCrumbPair(CachedEntity *player1, CachedEntity *player2,
                  std::pair<Vector, Vector> corners)
{
    Vector corner1 = corners.first;
    Vector corner2 = corners.second;

    {
        Vector dist       = corner1 - player1->m_vecOrigin();
        int maxiterations = floor(corner1.DistTo(player1->m_vecOrigin())) / 40;
        for (int i = 0; i < maxiterations; i++)
        {
            breadcrumbs.push_back(player1->m_vecOrigin() +
                                  dist / vectorMax(vectorAbs(dist)) * 40.0f *
                                      (i + 1));
        }
    }
    {
        Vector dist       = corner2 - corner1;
        int maxiterations = floor(corner2.DistTo(corner1)) / 40;
        for (int i = 0; i < maxiterations; i++)
        {
            breadcrumbs.push_back(corner1 + dist / vectorMax(vectorAbs(dist)) *
                                                40.0f * (i + 1));
        }
    }
    {
        Vector dist       = player2->m_vecOrigin() - corner2;
        int maxiterations = floor(corner2.DistTo(player2->m_vecOrigin())) / 40;
        for (int i = 0; i < maxiterations; i++)
        {
            breadcrumbs.push_back(corner2 + dist / vectorMax(vectorAbs(dist)) *
                                                40.0f * (i + 1));
        }
    }
}

int ClassPriority(CachedEntity *ent)
{
    switch (g_pPlayerResource->GetClass(ent))
    {
        if (g_pPlayerResource->GetClass(ent) == tf_spy)
            return 0;
    case tf_engineer:
        return 1;
    case tf_medic:
        return 2;
    case tf_pyro:
        return 3;
    case tf_scout:
        return 4;
    case tf_sniper:
        return 5;
    case tf_demoman:
        return 6;
    case tf_soldier:
        return 7;
    case tf_heavy:
        return 8;
    default:
        return 0;
    }
}
Timer waittime{};
int lastent = 0;
void WorldTick()
{
    if (!followbot)
    {
        follow_target = 0;
        return;
    }
    if (!inited)
        init();

    // We need a local player to control
    if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer())
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
        else if (CE_BAD(ENTITY(follow_target)))
            follow_target = 0;
    }

    if (!follow_target)
        breadcrumbs.clear(); // no target == no path
    // Target Selection
    if (steamid)
    {
        // Find a target with the steam id, as it is prioritized
        auto ent_count = HIGHEST_ENTITY;
        for (int i = 0; i < ent_count; i++)
        {
            auto entity = ENTITY(i);
            if (CE_BAD(entity)) // Exist + dormant
                continue;
            if (i == follow_target)
                break;
            if (entity->m_Type() != ENTITY_PLAYER)
                continue;
            if (steamid != entity->player_info.friendsID) // steamid check
                continue;

            if (!entity->m_bAlivePlayer()) // Dont follow dead players
                continue;
            if (corneractivate)
            {
                Vector indirectOrigin =
                    VischeckCorner(LOCAL_E, entity, float(follow_activation) / 2,
                                 true); // get the corner location that the
                                        // future target is visible from
                std::pair<Vector, Vector> corners;
                if (!indirectOrigin.z && entity->m_IDX == lastent)  // if we couldn't find it, run wallcheck instead
                {
                	corners = VischeckWall(LOCAL_E, entity, float(follow_activation) / 2, true);
                	if (!corners.first.z || !corners.second.z)
                		continue;
                    //addCrumbs(LOCAL_E, corners.first);
                    //addCrumbs(entity, corners.second);
                    addCrumbPair(LOCAL_E, entity, corners);
                }
                if (indirectOrigin.z)
                	addCrumbs(entity, indirectOrigin);
                else if (!indirectOrigin.z && !corners.first.z)
                	continue;
            }
            else
            {
                if (!VisCheckEntFromEnt(LOCAL_E, entity))
                    continue;
            }
            follow_target = entity->m_IDX;
            break;
        }
    }
    // If we dont have a follow target from that, we look again for someone
    // else who is suitable
    if ((!follow_target || change ||
         (ClassPriority(ENTITY(follow_target)) < 6 &&
          ENTITY(follow_target)->player_info.friendsID != steamid)) &&
        roambot)
    {
        // Try to get a new target
        auto ent_count =
            followcart ? HIGHEST_ENTITY : g_IEngine->GetMaxClients();
        for (int i = 0; i < ent_count; i++)
        {
            auto entity = ENTITY(i);
            if (CE_BAD(entity)) // Exist + dormant
                continue;
            if (!followcart)
                if (entity->m_Type() != ENTITY_PLAYER)
                    continue;
            if (entity == LOCAL_E) // Follow self lol
                continue;
            if (entity->m_bEnemy())
                continue;
            if (afk &&
                afkTicks[i].check(int(
                    afktime))) // don't follow target that was determined afk
                continue;
            if (IsPlayerDisguised(entity) || IsPlayerInvisible(entity))
                continue;
            if (!entity->m_bAlivePlayer()) // Dont follow dead players
                continue;
            if (follow_activation &&
                entity->m_flDistance() > (float) follow_activation)
                continue;
            const model_t *model =
                ENTITY(follow_target)->InternalEntity()->GetModel();
            if (followcart && model &&
                (lagexploit::pointarr[0] || lagexploit::pointarr[1] ||
                 lagexploit::pointarr[2] || lagexploit::pointarr[3] ||
                 lagexploit::pointarr[4]) &&
                (model == lagexploit::pointarr[0] ||
                 model == lagexploit::pointarr[1] ||
                 model == lagexploit::pointarr[2] ||
                 model == lagexploit::pointarr[3] ||
                 model == lagexploit::pointarr[4]))
                follow_target = entity->m_IDX;
            if (entity->m_Type() != ENTITY_PLAYER)
                continue;
            // favor closer entitys
            if (follow_target &&
                ENTITY(follow_target)->m_flDistance() <
                    entity->m_flDistance()) // favor closer entitys
                continue;
            // check if new target has a higher priority than current target
            if (ClassPriority(ENTITY(follow_target)) >=
                ClassPriority(ENTITY(i)))
                continue;

            if (corneractivate)
            {
                Vector indirectOrigin =
                    VischeckCorner(LOCAL_E, entity, 250,
                                   true); // get the corner location that the
                                          // future target is visible from
                std::pair<Vector, Vector> corners;
                corners.first.z  = 0;
                corners.second.z = 0;
                if (!indirectOrigin.z &&
                    entity->m_IDX == lastent) // if we couldn't find it, run
                                              // wallcheck instead
                {
                    corners = VischeckWall(LOCAL_E, entity, 250, true);
                    if (!corners.first.z || !corners.second.z)
                        continue;
                    addCrumbPair(LOCAL_E, entity, corners);
                }
                if (indirectOrigin.z)
                    addCrumbs(entity, indirectOrigin);
                else if (!indirectOrigin.z && !corners.first.z)
                    continue;
            }
            else
            {
                if (!VisCheckEntFromEnt(LOCAL_E, entity))
                    continue;
            }

            // ooooo, a target
            follow_target = i;
            afkTicks[i].update(); // set afk time to 0
        }
    }
    lastent++;
    if (lastent > g_IEngine->GetMaxClients())
    	lastent = 0;
    // last check for entity before we continue
    if (!follow_target)
        return;

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

    // If the player is close enough, we dont need to follow the path
    auto tar_orig       = followtar->m_vecOrigin();
    auto loc_orig       = LOCAL_E->m_vecOrigin();
    auto dist_to_target = loc_orig.DistTo(tar_orig);

    if ((dist_to_target < (float) follow_distance) &&
        VisCheckEntFromEnt(LOCAL_E, followtar))
    {
        idle_time.update();
    }

    // New crumbs, we add one if its empty so we have something to follow
    if ((breadcrumbs.empty() ||
         tar_orig.DistTo(breadcrumbs.at(breadcrumbs.size() - 1)) > 40.0F) &&
        DistanceToGround(ENTITY(follow_target)) < 45)
        breadcrumbs.push_back(tar_orig);

    // Prune old and close crumbs that we wont need anymore, update idle timer
    // too
    for (int i = 0; i < breadcrumbs.size(); i++)
    {
        if (loc_orig.DistTo(breadcrumbs.at(i)) < 60.f)
        {
            idle_time.update();
            for (int j = 0; j <= i; j++)
                breadcrumbs.erase(breadcrumbs.begin());
        }
    }

    // Tauntsync
    if (sync_taunt && HasCondition<TFCond_Taunting>(followtar) &&
        lastTaunt.test_and_set(1000))
    {
        g_IEngine->ClientCmd("taunt");
    }

    // Follow the crumbs when too far away, or just starting to follow
    if (dist_to_target > (float) follow_distance)
    {
        // Check for jump
        if (autojump && lastJump.check(1000) && (idle_time.check(2000) || DistanceToGround({breadcrumbs[0].x,breadcrumbs[0].y,breadcrumbs[0].z + 5}) > 47))
        {
            g_pUserCmd->buttons |= IN_JUMP;
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
        if (follow_target && (always_medigun || mimic_slot) &&
            (g_GlobalVars->curtime - last_slot_check > 1.0f) &&
            !g_pLocalPlayer->life_state &&
            !CE_BYTE(ENTITY(follow_target), netvar.iLifeState))
        {

            // We are checking our slot so reset the timer
            last_slot_check = g_GlobalVars->curtime;

            // Get the follow targets active weapon
            int owner_weapon_eid =
                (CE_INT(ENTITY(follow_target), netvar.hActiveWeapon) & 0xFFF);
            IClientEntity *owner_weapon =
                g_IEntityList->GetClientEntity(owner_weapon_eid);

            // If both the follow targets and the local players weapons arnt
            // null or
            // dormant
            if (owner_weapon && CE_GOOD(g_pLocalPlayer->weapon()))
            {

                // IsBaseCombatWeapon()
                if (re::C_BaseCombatWeapon::IsBaseCombatWeapon(
                        RAW_ENT(g_pLocalPlayer->weapon())) &&
                    re::C_BaseCombatWeapon::IsBaseCombatWeapon(owner_weapon))
                {

                    // Get the players slot numbers and store in some vars
                    int my_slot = re::C_BaseCombatWeapon::GetSlot(
                        RAW_ENT(g_pLocalPlayer->weapon()));
                    int owner_slot =
                        re::C_BaseCombatWeapon::GetSlot(owner_weapon);

                    // If the local player is a medic and user settings allow,
                    // then
                    // keep the medigun out
                    if (g_pLocalPlayer->clazz == tf_medic && always_medigun)
                    {
                        if (my_slot != 1)
                        {
                            g_IEngine->ExecuteClientCmd("slot2");
                        }

                        // Else we attemt to keep our weapon mimiced with our
                        // follow
                        // target
                    }
                    else
                    {
                        if (my_slot != owner_slot)
                        {
                            g_IEngine->ExecuteClientCmd(
                                format("slot", owner_slot + 1).c_str());
                        }
                    }
                }
            }
        }
        WalkTo(breadcrumbs[0]);
    }
    else
        idle_time.update();
}

void DrawTick()
{
#if ENABLE_VISUALS
    if (!followbot || !draw_crumb)
        return;
    if (breadcrumbs.size() < 2)
        return;
    for (size_t i = 0; i < breadcrumbs.size() - 1; i++)
    {
        Vector wts1, wts2;
        if (draw::WorldToScreen(breadcrumbs[i], wts1) &&
            draw::WorldToScreen(breadcrumbs[i + 1], wts2))
        {
            glez::draw::line(wts1.x, wts1.y, wts2.x - wts1.x, wts2.y - wts1.y,
                             colors::white, 0.1f);
        }
    }
    Vector wts;
    if (!draw::WorldToScreen(breadcrumbs[0], wts))
        return;
    glez::draw::rect(wts.x - 4, wts.y - 4, 8, 8, colors::white);
    glez::draw::rect_outline(wts.x - 4, wts.y - 4, 7, 7, colors::white, 1.0f);
#endif
}

#if ENABLE_IPC
static CatCommand
    follow_me("fb_follow_me", "IPC connected bots will follow you", []() {
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
        std::string tmp =
            CON_PREFIX + follow_steam.name + " " + std::to_string(steam_id);
        if (tmp.length() >= 63)
        {
            ipc::peer->SendMessage(0, 0, ipc::commands::execute_client_cmd_long,
                                   tmp.c_str(), tmp.length() + 1);
        }
        else
        {
            ipc::peer->SendMessage(tmp.c_str(), 0,
                                   ipc::commands::execute_client_cmd, 0, 0);
        }
    });
#endif
}
