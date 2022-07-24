/*
 * hoovy.cpp
 *
 *  Created on: Apr 14, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

static bool hoovy_list[PLAYER_ARRAY_SIZE] = { 0 };

bool HasSandvichOut(CachedEntity *entity)
{

    int weapon_idx;
    CachedEntity *weapon;

    weapon_idx = CE_INT(entity, netvar.hActiveWeapon) & 0xFFF;
    if (!(weapon_idx > 0 && weapon_idx <= HIGHEST_ENTITY))
        return false;
    weapon = ENTITY(weapon_idx);
    if (CE_GOOD(weapon))
    {
        if (weapon->m_iClassID() == CL_CLASS(CTFLunchBox))
        {
            return true;
        }
    }
    return false;
}

bool IsHoovyHelper(CachedEntity *entity)
{
    if (HasSandvichOut(entity) && (CE_INT(entity, netvar.iFlags) & FL_DUCKING))
        return true;
    return false;
}

void UpdateHoovyList()
{
    if (CE_BAD(LOCAL_E))
        return;

    static CachedEntity *ent;
    for (int i = 1; i <= MAX_PLAYERS; i++)
    {
        ent = ENTITY(i);
        if (CE_GOOD(ent) && CE_BYTE(ent, netvar.iLifeState) == LIFE_ALIVE)
        {
            if (!hoovy_list[i - 1])
            {
                if (IsHoovyHelper(ent))
                    hoovy_list[i - 1] = true;
            }
            else
            {
                if (!HasSandvichOut(ent))
                    hoovy_list[i - 1] = false;
            }
        }
    }
}

bool IsHoovy(CachedEntity *entity)
{
    if (!entity->m_IDX || entity->m_IDX > 32)
        return false;
    return hoovy_list[entity->m_IDX - 1];
}

static InitRoutine init_heavy([]() { EC::Register(EC::CreateMove, UpdateHoovyList, "cm_hoovylist", EC::average); });
