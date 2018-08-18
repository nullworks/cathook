/*
 * bhop.cpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#include <hacks/AntiCheat.hpp>
#include <settings/Int.hpp>
#include "common.hpp"

static settings::Int bhop_detect_count{ "find-cheaters.bunnyhop.detections",
                                        "4" };

namespace ac::bhop
{

ac_data data_table[32]{};

void ResetEverything()
{
    memset(data_table, 0, sizeof(ac_data) * 32);
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
    auto &data  = data_table[player->m_IDX - 1];
    bool ground = player->var<int>(netvar.iFlags) & FL_ONGROUND;
    if (ground)
    {
        if (!data.was_on_ground)
        {
            data.ticks_on_ground = 1;
        }
        else
        {
            data.ticks_on_ground++;
        }
    }
    else
    {
        if (data.was_on_ground)
        {
            if (data.ticks_on_ground == 1)
            {
                data.detections++;
                // TODO FIXME
                if (data.detections >= int(bhop_detect_count))
                {
                    logging::Info("[%d] Suspected BHop: %d", player->m_IDX,
                                  data.detections);
                    if ((tickcount - data.last_accusation) > 600)
                    {
                        hacks::shared::anticheat::Accuse(
                            player->m_IDX, "Bunnyhop",
                            format("Perfect jumps = ", data.detections));
                        data.last_accusation = tickcount;
                    }
                }
            }
            else
            {
                data.detections = 0;
            }
        }
        data.ticks_on_ground = 0;
    }
    data.was_on_ground = ground;
}

void Event(KeyValues *event)
{
}
} // namespace ac::bhop
