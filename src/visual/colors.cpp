/*
 * colors.cpp
 *
 *  Created on: May 22, 2017
 *      Author: nullifiedcat
 */

#include <online/Online.hpp>
#include <PlayerTools.hpp>
#include "common.hpp"

rgba_t colors::EntityF(CachedEntity *ent)
{
    rgba_t result, plclr;
    int skin;
    k_EItemType type;

    using namespace colors;
    result = white;
    type   = ent->m_ItemType();
    if (type)
    {
        if ((type >= ITEM_HEALTH_SMALL && type <= ITEM_HEALTH_LARGE) ||
            type == ITEM_TF2C_PILL)
            result = green;
        else if (type >= ITEM_POWERUP_FIRST && type <= ITEM_POWERUP_LAST)
        {
            skin = RAW_ENT(ent)->GetSkin();
            if (skin == 1)
                result = red;
            else if (skin == 2)
                result = blu;
            else
                result = yellow;
        }
        else if (type >= ITEM_TF2C_W_FIRST && type <= ITEM_TF2C_W_LAST)
        {
            if (CE_BYTE(ent, netvar.bRespawning))
            {
                result = red;
            }
            else
            {
                result = yellow;
            }
        }
        else if (type == ITEM_HL_BATTERY)
        {
            result = yellow;
        }
    }

    if (ent->m_iClassID() == CL_CLASS(CCurrencyPack))
    {
        if (CE_BYTE(ent, netvar.bDistributed))
            result = red;
        else
            result = green;
    }

    if (ent->m_Type() == ENTITY_PROJECTILE)
    {
        if (ent->m_iTeam() == TEAM_BLU)
            result = blu;
        else if (ent->m_iTeam() == TEAM_RED)
            result = red;
        if (ent->m_bCritProjectile())
        {
            if (ent->m_iTeam() == TEAM_BLU)
                result = blu_u;
            else if (ent->m_iTeam() == TEAM_RED)
                result = red_u;
        }
    }

    if (ent->m_Type() == ENTITY_PLAYER || ent->m_Type() == ENTITY_BUILDING)
    {
        if (ent->m_iTeam() == TEAM_BLU)
            result = blu;
        else if (ent->m_iTeam() == TEAM_RED)
            result = red;
        if (ent->m_Type() == ENTITY_PLAYER)
        {
            if (IsPlayerInvulnerable(ent))
            {
                if (ent->m_iTeam() == TEAM_BLU)
                    result = blu_u;
                else if (ent->m_iTeam() == TEAM_RED)
                    result = red_u;
            }
            if (HasCondition<TFCond_UberBulletResist>(ent))
            {
                if (ent->m_iTeam() == TEAM_BLU)
                    result = blu_v;
                else if (ent->m_iTeam() == TEAM_RED)
                    result = red_v;
            }

            auto o = player_tools::forceEspColor(ent);
            if (o.has_value())
                return *o;
        }
    }

    return result;
}

rgba_t colors::RainbowCurrent()
{
    return colors::FromHSL(fabs(sin(g_GlobalVars->curtime / 2.0f)) * 360.0f,
                           0.85f, 0.9f);
}

static unsigned char hexToChar(char i)
{
    if (i >= '0' && i <= '9')
        return i - '0';
    if (i >= 'a' && i <= 'f')
        return i - 'a' + 10;
    if (i >= 'A' && i <= 'F')
        return i - 'A' + 10;
    return 0;
}

static unsigned int hexToByte(char hi, char lo)
{
    return (hexToChar(hi) << 4) | (hexToChar(lo));
}

colors::rgba_t::rgba_t(const char hex[6])
{
    auto ri = hexToByte(hex[0], hex[1]);
    auto gi = hexToByte(hex[2], hex[3]);
    auto bi = hexToByte(hex[4], hex[5]);
    r       = float(ri) / 255.0f;
    g       = float(gi) / 255.0f;
    b       = float(bi) / 255.0f;
    a       = 1.0f;
}
