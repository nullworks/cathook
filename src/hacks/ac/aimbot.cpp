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

static settings::Bool enable{ "find-cheaters.aimbot.enable", "true" };
static settings::Float detect_angle{ "find-cheaters.aimbot.angle", "30" };
static settings::Int detections_warning{ "find-cheaters.aimbot.detections",
                                         "3" };

namespace ac::aimbot
{

ac_data data_table[32];
int amount[32];

void ResetEverything()
{
    memset(&data_table, 0, sizeof(ac_data) * 32);
}

void ResetPlayer(int idx)
{
    memset(&data_table[idx - 1], 0, sizeof(ac_data));
}

void Init()
{
    ResetEverything();
}

void Update(CachedEntity *player)
{
    if (!enable)
        return;
    auto &data = data_table[player->m_IDX - 1];
    auto &am   = amount[player->m_IDX - 1];
    if (data.check_timer)
    {
        data.check_timer--;
        if (!data.check_timer)
        {
            auto &angles      = angles::data(player);
            float deviation   = angles.deviation(2);
            int widx          = CE_INT(player, netvar.hActiveWeapon) & 0xFFF;
            CachedEntity *wep = ENTITY(widx);
            if (!CE_GOOD(wep))
                return;
            if (deviation > float(detect_angle) &&
                wep->m_iClassID() != CL_CLASS(CTFFlameThrower))
            {
                am++;
                // logging::Info("[ac] %d deviation %.2f #%d", player->m_IDX,
                // deviation, data.detections);
                player_info_t info;
                g_IEngine->GetPlayerInfo(player->m_IDX, &info);
                if (am > 5)
                {
                    hacks::shared::anticheat::SetRage(info);
                    am = 0;
                }
                if (++data.detections > int(detections_warning))
                {

                    const char *wp_name = "[unknown]";
                    int widx = CE_INT(player, netvar.hActiveWeapon) & 0xFFF;
                    if (IDX_GOOD(widx))
                    {
                        CachedEntity *weapon = ENTITY(widx);
                        wp_name              = weapon->InternalEntity()
                                      ->GetClientClass()
                                      ->GetName();
                        /*logging::Info("%d", weapon->m_IDX);
                        logging::Info("%s", );
                        IClientEntity* e_weapon = RAW_ENT(weapon);
                        if (CE_GOOD(weapon)) {
                            const char* wname = vfunc<const
                        char*(*)(IClientEntity*)>(e_weapon, 398, 0)(e_weapon);
                            if (wname) wp_name = wname;
                        }*/
                    }
                    hacks::shared::anticheat::Accuse(
                        player->m_IDX, "Aimbot",
                        format("Weapon: ", wp_name, " | Deviation: ", deviation,
                               "° | ", data.detections));
                }
            }
        }
    }
}

void Event(KeyValues *event)
{
    if (!enable)
        return;
    if (!strcmp(event->GetName(), "player_death") ||
        !strcmp(event->GetName(), "player_hurt"))
    {
        int attacker = event->GetInt("attacker");
        int victim   = event->GetInt("userid");
        int eid      = g_IEngine->GetPlayerForUserID(attacker);
        int vid      = g_IEngine->GetPlayerForUserID(victim);
        if (eid > 0 && eid < 33)
        {
            CachedEntity *victim   = ENTITY(vid);
            CachedEntity *attacker = ENTITY(eid);
            if (CE_GOOD(victim) && CE_GOOD(attacker))
                if (victim->m_vecOrigin().DistTo(attacker->m_vecOrigin()) > 250)
                {
                    data_table[eid - 1].check_timer = 1;
                    data_table[eid - 1].last_weapon = event->GetInt("weaponid");
                }
        }
    }
}
} // namespace ac::aimbot
