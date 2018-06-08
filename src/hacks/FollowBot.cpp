
/*
 * CREDITS TO ONEE-CHAN FOR IMPROVED FOLLOWBOT
 *
 *
 *
 */

#include "common.hpp"
#include <hacks/FollowBot.hpp>
#include <hacks/LagExploit.hpp>

namespace hacks
{
namespace shared
{
namespace followbot
{

CatVar followbot(CV_SWITCH, XORSTR("fb"), XORSTR("0"), XORSTR("Followbot Switch"),
                 XORSTR("Set to 1 in followbots' configs"));
bool followcart = false;
CatVar roambot(CV_SWITCH, XORSTR("fb_roaming"), XORSTR("1"), XORSTR("Roambot"),
               XORSTR("Followbot will roam free, finding targets it can"));
static CatVar draw_crumb(CV_SWITCH, XORSTR("fb_draw"), XORSTR("1"), XORSTR("Draw crumbs"),
                         XORSTR("Self explanitory"));
static CatVar follow_distance(CV_INT, XORSTR("fb_distance"), XORSTR("175"), XORSTR("Follow Distance"),
                              XORSTR("How close the bots should stay to the target"));
static CatVar follow_activation(CV_INT, XORSTR("fb_activation"), XORSTR("175"),
                                XORSTR("Activation Distance"),
                                XORSTR("How close a player should be until the "
                                "followbot will pick them as a target"));
unsigned steamid = 0x0;
CatCommand follow_steam(XORSTR("fb_steam"), XORSTR("Follow Steam Id"),
                        [](const CCommand &args) {
                            if (args.ArgC() < 1)
                            {
                                steamid = 0x0;
                                return;
                            }
                            unsigned tempid = atol(args.Arg(1));
                            steamid         = *(unsigned int *) &tempid;

                        });
static CatVar mimic_slot(CV_SWITCH, XORSTR("fb_mimic_slot"), XORSTR("0"), XORSTR("Mimic weapon slot"),
                         XORSTR("Mimic follow target's weapon slot"));
static CatVar always_medigun(CV_SWITCH, XORSTR("fb_always_medigun"), XORSTR("0"),
                             XORSTR("Always Medigun"), XORSTR("Always use medigun"));
static CatVar sync_taunt(CV_SWITCH, XORSTR("fb_sync_taunt"), XORSTR("0"), XORSTR("Synced taunt"),
                         XORSTR("Taunt when follow target does"));
static CatVar change(CV_SWITCH, XORSTR("fb_switch"), XORSTR("1"), XORSTR("Change followbot target"),
                     XORSTR("Always change roaming target when possible"));
// Something to store breadcrumbs created by followed players
static std::vector<Vector> breadcrumbs;
static const int crumb_limit = 64; // limit

// Followed entity, externed for highlight color
int follow_target = 0;

void WorldTick()
{
    if (!followbot)
    {
        follow_target = 0;
        return;
    }

    // We need a local player to control
    if (CE_BAD(LOCAL_E))
    {
        follow_target = 0;
        return;
    }

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
            if (entity->m_Type() != ENTITY_PLAYER)
                continue;
            if (steamid != entity->player_info.friendsID) // steamid check
                continue;
            logging::Info(XORSTR("Success"));

            if (!entity->m_bAlivePlayer()) // Dont follow dead players
                continue;
            if (!VisCheckEntFromEnt(LOCAL_E, entity))
                continue;
            follow_target = entity->m_IDX;
            break;
        }
    }
    // If we dont have a follow target from that, we look again for someone
    // else who is suitable
    if ((!follow_target || change) && roambot)
    {
        // Try to get a new target
        auto ent_count = HIGHEST_ENTITY;
        for (int i = 0; i < HIGHEST_ENTITY; i++)
        {
            auto entity = ENTITY(i);
            if (CE_BAD(entity)) // Exist + dormant
                continue;
            if (!followcart)
                if (entity->m_Type() != ENTITY_PLAYER)
                    continue;
            if (entity == LOCAL_E) // Follow self lol
                continue;
            if (!entity->m_bAlivePlayer()) // Dont follow dead players
                continue;
            if (follow_activation &&
                entity->m_flDistance() > (float) follow_activation)
                continue;
            if (!VisCheckEntFromEnt(LOCAL_E, entity))
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
            if (follow_target &&
                ENTITY(follow_target)->m_flDistance() >
                    entity->m_flDistance()) // favor closer entitys
                continue;
            // ooooo, a target
            follow_target = entity->m_IDX;
        }
    }
    // last check for entity before we continue
    if (!follow_target)
        return;

    // If the player is close enough, we dont need to follow the path
    CachedEntity *followtar = ENTITY(follow_target);
    // wtf is this needed
    if (CE_BAD(followtar))
        return;
    auto tar_orig       = followtar->m_vecOrigin();
    auto loc_orig       = LOCAL_E->m_vecOrigin();
    auto dist_to_target = loc_orig.DistTo(tar_orig);
    if (dist_to_target < 30)
        breadcrumbs.clear();

    // Update timer on new target
    static Timer idle_time{};
    if (breadcrumbs.empty())
        idle_time.update();

    // New crumbs, we add one if its empty so we have something to follow
    if ((breadcrumbs.empty() ||
         tar_orig.DistTo(breadcrumbs.at(breadcrumbs.size() - 1)) > 40.0F) &&
        DistanceToGround(ENTITY(follow_target)) < 30)
        breadcrumbs.push_back(tar_orig);

    // Prune old and close crumbs that we wont need anymore, update idle timer
    // too
    while (breadcrumbs.size() > 1 && loc_orig.DistTo(breadcrumbs.at(0)) < 60.f)
    {
        idle_time.update();
        breadcrumbs.erase(breadcrumbs.begin());
    }

    // Follow the crumbs when too far away, or just starting to follow
    if (dist_to_target > (float) follow_distance)
    {

        // Check for idle
        if (idle_time.test_and_set(3000))
        {
            follow_target = 0;
            return;
        }
        if (sync_taunt && HasCondition<TFCond_Taunting>(ENTITY(follow_target)))
            g_IEngine->ClientCmd(XORSTR("taunt"));
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
                            g_IEngine->ExecuteClientCmd(XORSTR("slot2"));
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
                                format(XORSTR("slot"), owner_slot + 1).c_str());
                        }
                    }
                }
            }
        }
        WalkTo(breadcrumbs.at(0));
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
        if (draw::WorldToScreen(breadcrumbs.at(i), wts1) &&
            draw::WorldToScreen(breadcrumbs.at(i + 1), wts2))
        {
            draw_api::draw_line(wts1.x, wts1.y, wts2.x - wts1.x,
                                wts2.y - wts1.y, colors::white, 0.1f);
        }
    }
    Vector wts;
    if (!draw::WorldToScreen(breadcrumbs.at(0), wts))
        return;
    draw_api::draw_rect(wts.x - 4, wts.y - 4, 8, 8, colors::white);
    draw_api::draw_rect_outlined(wts.x - 4, wts.y - 4, 7, 7, colors::white,
                                 1.0f);
#endif
}

#if ENABLE_IPC
static CatCommand
    follow_me(XORSTR("fb_follow_me"), XORSTR("IPC connected bots will follow you"), []() {
        if (!ipc::peer)
        {
            logging::Info(XORSTR("IPC isnt connected"));
            return;
        }
        auto local_ent = LOCAL_E;
        if (!local_ent)
        {
            logging::Info(XORSTR("Cant get a local player"));
            return;
        }
        player_info_s info;
        g_IEngine->GetPlayerInfo(local_ent->m_IDX, &info);
        auto steam_id = info.friendsID;
        if (!steam_id)
        {
            logging::Info(XORSTR("Cant get steam-id, the game module probably doesnt "
                          "support it."));
            return;
        }
        // Construct the command
        std::string tmp =
            CON_PREFIX + follow_steam.name + XORSTR(" ") + std::to_string(steam_id);
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
}
}
