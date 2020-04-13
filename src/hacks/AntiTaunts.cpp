/*
 * AntiTaunts.cpp
 *
 *  Created on: Apr 12, 2020
 */

#include <settings/Bool.hpp>
#include "common.hpp"

namespace hacks::tf2::antitaunts
{
static settings::Boolean remove_taunts{ "remove.taunts", "false" };

void cm()
{
    CachedEntity *ent;
    if (!remove_taunts)
        return;

    for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
    {
        ent = ENTITY(i);
        if (CE_BAD(ent) || ent == LOCAL_E || ent->m_Type() != ENTITY_PLAYER)
        {
            continue;
        }
        RemoveCondition<TFCond_Taunting>(ent);
    }
}
static InitRoutine EC([]() { EC::Register(EC::CreateMove, cm, "antitaunts", EC::average); });
} // namespace hacks::tf2::antitaunts
