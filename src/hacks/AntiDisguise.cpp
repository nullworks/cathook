/*
 * AntiDisguise.cpp
 *
 *  Created on: Nov 16, 2016
 *      Author: nullifiedcat
 */

#include <settings/Bool.hpp>
#include "common.hpp"

namespace hacks::tf2::antidisguise
{
static settings::Boolean enable{ "remove.disguise", "0" };
static settings::Boolean no_invisibility{ "remove.cloak", "0" };

void cm()
{
    CachedEntity *ent;
    if (!*enable && !*no_invisibility)
        return;

    for (int i = 1; i <= g_IEngine->GetMaxClients(); i++)
    {
        ent = ENTITY(i);
        if (CE_BAD(ent) || ent == LOCAL_E || ent->m_Type() != ENTITY_PLAYER || CE_INT(ent, netvar.iClass) != tf_class::tf_spy)
        {
            continue;
        }
        if (*enable)
            RemoveCondition<TFCond_Disguised>(ent);
        if (*no_invisibility)
        {
            RemoveCondition<TFCond_Cloaked>(ent);
            RemoveCondition<TFCond_CloakFlicker>(ent);
        }
    }
}
static InitRoutine EC([]() { EC::Register(EC::CreateMove, cm, "antidisguise", EC::average); });
} // namespace hacks::tf2::antidisguise
