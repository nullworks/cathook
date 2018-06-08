/*
 * colors.cpp
 *
 *  Created on: May 22, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

static CatVar user_red_blue(CV_INT, XORSTR("esp_color_red_b"), XORSTR("0"), XORSTR("Red Team: Blue"),
                            XORSTR("Blue color for red team"), 255);
static CatVar user_red_green(CV_INT, XORSTR("esp_color_red_g"), XORSTR("0"), XORSTR("Red Team: Green"),
                             XORSTR("Green color for red team"), 255);
static CatVar user_red_red(CV_INT, XORSTR("esp_color_red_r"), XORSTR("0"), XORSTR("Red Team: Red"),
                           XORSTR("Red color for red team"), 255);

static CatVar user_blue_blue(CV_INT, XORSTR("esp_color_blue_b"), XORSTR("0"), XORSTR("Blue Team: Blue"),
                             XORSTR("Blue color for blue team"), 255);
static CatVar user_blue_green(CV_INT, XORSTR("esp_color_blue_g"), XORSTR("0"),
                              XORSTR("Blue Team: Green"), XORSTR("Green color for blue team"),
                              255);
static CatVar user_blue_red(CV_INT, XORSTR("esp_color_blue_r"), XORSTR("0"), XORSTR("Blue Team: Red"),
                            XORSTR("Red color for blue team"), 255);

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
        // If user has custom color, check if we should change, and do so here
        if (user_red_blue || user_red_green || user_red_red || user_blue_blue ||
            user_blue_green || user_blue_red)
        {
            switch (ent->m_iTeam())
            {
            case TEAM_BLU:
                if (user_blue_blue || user_blue_green || user_blue_red)
                {
                    result = FromRGBA8(user_blue_red, user_blue_green,
                                       user_blue_blue, 255);
                }
                break;
            case TEAM_RED:
                if (user_red_blue || user_red_green || user_red_red)
                {
                    result = FromRGBA8(user_red_red, user_red_green,
                                       user_red_blue, 255);
                }
                break;
            }
        }
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
            plclr = playerlist::Color(ent);
            if (plclr.a)
                result = plclr;
        }
    }

    return result;
}

rgba_t colors::RainbowCurrent()
{
    return colors::FromHSL(fabs(sin(g_GlobalVars->curtime / 2.0f)) * 360.0f,
                           0.85f, 0.9f);
}
