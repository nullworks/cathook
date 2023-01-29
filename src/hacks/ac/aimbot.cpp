/*
 * aimbot.cpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#include <hacks/ac/aimbot.hpp>
#include <hacks/AntiCheat.hpp>
#include <settings/Float.hpp>
#include "common.hpp"
#include "angles.hpp"

namespace ac::aimbot
{
static settings::Boolean enable{ "find-cheaters.aimbot.enable", "true" };
static settings::Float detect_angle{ "find-cheaters.aimbot.angle", "30" };
static settings::Int detections_warning{ "find-cheaters.aimbot.detections", "3" };

ac_data data_table[MAX_PLAYERS];
int amount[MAX_PLAYERS];
boost::unordered_flat_map<int, Vector> Player_origs{};

boost::unordered_flat_map<int, Vector> &player_orgs()
{
    return Player_origs;
}
void ResetEverything()
{
    memset(&data_table, 0, sizeof(ac_data) * MAX_PLAYERS);
    Player_origs.clear();
}

void ResetPlayer(int idx)
{
    memset(&data_table[idx - 1], 0, sizeof(ac_data));
    Player_origs.clear();
}

void Init()
{
    ResetEverything();
}

void Update(CachedEntity *player)
{
    if (!enable)
        return;
    Player_origs[player->m_IDX] = player->m_vecOrigin();
    auto &data                  = data_table[player->m_IDX - 1];
    auto &am                    = amount[player->m_IDX - 1];
    if (data.check_timer)
    {
        data.check_timer--;
        if (!data.check_timer)
        {
            auto &angles      = angles::data_[player->m_IDX];
            float deviation   = angles.deviation(2);
            int widx          = HandleToIDX(CE_INT(player, netvar.hActiveWeapon));
            CachedEntity *wep = ENTITY(widx);
            if (!CE_GOOD(wep))
                return;
            if (deviation > float(detect_angle) && wep->m_iClassID() != CL_CLASS(CTFFlameThrower))
            {
                am++;
                // logging::Info("[ac] %d deviation %.2f #%d", player->m_IDX,
                // deviation, data.detections);
                player_info_t info;
                GetPlayerInfo(player->m_IDX, &info);
                if (am > 5)
                {
                    hacks::shared::anticheat::SetRage(info);
                    am = 0;
                }
                if (++data.detections > int(detections_warning))
                {

                    const char *wp_name = "[unknown]";
                    int widx            = HandleToIDX(CE_INT(player, netvar.hActiveWeapon));
                    if (IDX_GOOD(widx))
                    {
                        CachedEntity *weapon = ENTITY(widx);
                        wp_name              = weapon->InternalEntity()->GetClientClass()->GetName();
                        /*logging::Info("%d", weapon->m_IDX);
                        logging::Info("%s", );
                        IClientEntity* e_weapon = RAW_ENT(weapon);
                        if (CE_GOOD(weapon)) {
                            const char* wname = vfunc<const
                        char*(*)(IClientEntity*)>(e_weapon, 398, 0)(e_weapon);
                            if (wname) wp_name = wname;
                        }*/
                    }
                    hacks::shared::anticheat::Accuse(player->m_IDX, "Aimbot", format("Weapon: ", wp_name, " | Deviation: ", deviation, "° | ", data.detections));
                }
            }
        }
    }
}

void Event(KeyValues *event)
{
    if (!enable)
        return;
    if (!strcmp(event->GetName(), "player_death") || !strcmp(event->GetName(), "player_hurt"))
    {
        int attacker = event->GetInt("attacker");
        int victim   = event->GetInt("userid");
        int eid      = GetPlayerForUserID(attacker);
        int vid      = GetPlayerForUserID(victim);
        if (eid > 0 && eid <= MAX_PLAYERS && vid > 0 && vid <= MAX_PLAYERS)
        {
            auto &Po_v = Player_origs[vid];
            auto &Po_e = Player_origs[eid];
            if (Po_v.z != 0 && Po_e.z != 0)
                if (Po_v.DistTo(Po_e) > 250)
                {
                    data_table[eid - 1].check_timer = 1;
                    data_table[eid - 1].last_weapon = event->GetInt("weaponid");
                }
        }
    }
}
} // namespace ac::aimbot
