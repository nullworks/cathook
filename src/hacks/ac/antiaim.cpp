/*
 * antiaim.cpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#include <hacks/AntiCheat.hpp>
#include <settings/Bool.hpp>
#include "common.hpp"
#include "angles.hpp"

namespace ac::antiaim
{
static settings::Boolean enable{ "find-cheaters.antiaim.enable", "true" };

unsigned long last_accusation[MAX_PLAYERS]{ 0 };

void ResetEverything()
{
    memset(last_accusation, 0, sizeof(unsigned long) * MAX_PLAYERS);
}

void ResetPlayer(int idx)
{
    last_accusation[idx - 1] = 0;
}

int amount[MAX_PLAYERS] = {};

void Update(CachedEntity *player)
{
    if (!enable)
        return;
    auto &am = amount[player->m_IDX - 1];
    if (tickcount - last_accusation[player->m_IDX - 1] < 60 * 60)
        return;
    const auto &d = angles::data(player);
    if (d.angle_count)
    {
        int idx = d.angle_index - 1;
        if (idx < 0)
            idx = d.count - 1;
        if ((d.angles[idx].x < -89 || d.angles[idx].x > 89) && (d.angles[idx].x < 89.2941 || d.angles[idx].x > 89.2942))
        {
            am++;
            player_info_t info;
            g_IEngine->GetPlayerInfo(player->m_IDX, &info);
            if (am > 5)
            {
                hacks::shared::anticheat::SetRage(info);
                am = 0;
            }
            std::string reason = format("Pitch: ", d.angles[idx].x, " Yaw: ", d.angles[idx].y);
            if (d.angles[idx].x == -271.0f)
            {
                reason += " (Fakeup)";
            }
            else if (d.angles[idx].x == 271.0f)
            {
                reason += " (Fakedown)";
            }
            hacks::shared::anticheat::Accuse(player->m_IDX, "AntiAim", reason);
            last_accusation[player->m_IDX - 1] = tickcount;
        }
    }
}
} // namespace ac::antiaim
