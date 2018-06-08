/*
 * aimbot.cpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#include <hacks/ac/aimbot.hpp>
#include <hacks/AntiCheat.hpp>
#include "common.hpp"

namespace ac
{
namespace aimbot
{

static CatVar enabled(CV_SWITCH, XORSTR("ac_aimbot"), XORSTR("1"), XORSTR("Detect Aimbot"),
                      XORSTR("Is not recommended"));
static CatVar detect_angle(CV_FLOAT, XORSTR("ac_aimbot_angle"), XORSTR("30"), XORSTR("Aimbot Angle"));
static CatVar detections_warning(CV_INT, XORSTR("ac_aimbot_detections"), XORSTR("3"),
                                 XORSTR("Min detections to warn"));

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
    if (not enabled)
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
                // logging::Info(XORSTR("[ac] %d deviation %.2f #%d"), player->m_IDX,
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

                    const char *wp_name = XORSTR("[unknown]");
                    int widx = CE_INT(player, netvar.hActiveWeapon) & 0xFFF;
                    if (IDX_GOOD(widx))
                    {
                        CachedEntity *weapon = ENTITY(widx);
                        wp_name              = weapon->InternalEntity()
                                      ->GetClientClass()
                                      ->GetName();
                        /*logging::Info(XORSTR("%d"), weapon->m_IDX);
                        logging::Info(XORSTR("%s"), );
                        IClientEntity* e_weapon = RAW_ENT(weapon);
                        if (CE_GOOD(weapon)) {
                            const char* wname = vfunc<const
                        char*(*)(IClientEntity*)>(e_weapon, 398, 0)(e_weapon);
                            if (wname) wp_name = wname;
                        }*/
                    }
                    hacks::shared::anticheat::Accuse(
                        player->m_IDX, XORSTR("Aimbot"),
                        format(XORSTR("Weapon: "), wp_name, XORSTR(" | Deviation: "), deviation,
                               XORSTR("° | "), data.detections));
                }
            }
        }
    }
}

void Event(KeyValues *event)
{
    if (not enabled)
        return;
    if (!strcmp(event->GetName(), XORSTR("player_death")) ||
        !strcmp(event->GetName(), XORSTR("player_hurt")))
    {
        int attacker = event->GetInt(XORSTR("attacker"));
        int victim   = event->GetInt(XORSTR("userid"));
        int eid      = g_IEngine->GetPlayerForUserID(attacker);
        int vid      = g_IEngine->GetPlayerForUserID(victim);
        if (eid > 0 && eid < 33)
        {
            CachedEntity *victim   = ENTITY(vid);
            CachedEntity *attacker = ENTITY(eid);
            if (victim->m_vecOrigin().DistTo(attacker->m_vecOrigin()) > 250)
            {
                data_table[eid - 1].check_timer = 1;
                data_table[eid - 1].last_weapon = event->GetInt(XORSTR("weaponid"));
            }
        }
    }
}
}
}
