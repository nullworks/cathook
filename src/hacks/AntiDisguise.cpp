/*
 * AntiDisguise.cpp
 *
 *  Created on: Nov 16, 2016
 *      Author: nullifiedcat
 */

#include <settings/Bool.hpp>
#include "common.hpp"

static settings::Bool enable{ "remove.disguise", "0" };
static settings::Bool no_invisibility{ "remove.cloak", "0" };

namespace hacks::tf2::antidisguise
{

void Draw()
{
    CachedEntity *ent;
    if (no_invisibility)
        for (int i = 0; i < g_IEngine->GetMaxClients(); i++)
        {
            ent = ENTITY(i);
            if (CE_BAD(ent))
                continue;
            if (ent->m_Type() == ENTITY_PLAYER)
            {
                if (CE_INT(ent, netvar.iClass) == tf_class::tf_spy)
                {
                    RemoveCondition<TFCond_Cloaked>(ent);
                    RemoveCondition<TFCond_CloakFlicker>(ent);
                }
            }
        }
    if (!enable)
        return;
    for (int i = 0; i < g_IEngine->GetMaxClients(); i++)
    {
        ent = ENTITY(i);
        if (CE_BAD(ent))
            continue;
        if (ent->m_Type() == ENTITY_PLAYER)
        {
            if (CE_INT(ent, netvar.iClass) == tf_class::tf_spy)
            {
                RemoveCondition<TFCond_Disguised>(ent);
            }
        }
    }
}
} // namespace hacks::tf2::antidisguise
