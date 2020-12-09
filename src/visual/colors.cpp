/*
 * colors.cpp
 *
 *  Created on: May 22, 2017
 *      Author: nullifiedcat
 */

#include <PlayerTools.hpp>
#include "common.hpp"

namespace colors
{
settings::Rgba col_red{ "colors.team-red", "ed2a2aff" };
settings::Rgba col_blu{ "colors.team-blu", "1c6cedff" };
settings::Rgba col_red_b{ "colors.team-red.background", "402020ff" };
settings::Rgba col_blu_b{ "colors.team-blu.background", "202040ff" };
settings::Rgba col_red_v{ "colors.team-red.vaccinator", "c4666cff" };
settings::Rgba col_blu_v{ "colors.team-blu.vaccinator", "66b6c4ff" };
settings::Rgba col_red_u{ "colors.team-red.ubercharge", "ff6600ff" };
settings::Rgba col_blu_u{ "colors.team-blu.ubercharge", "003399ff" };
settings::Rgba col_guicolor{ "colors.guicolor", "ffffffff" };
settings::Rgba col_target{ "colors.target", "00ff00ff" };

rgba_t red   = *col_red;
rgba_t blu   = *col_blu;
rgba_t red_b = *col_red_b;
rgba_t blu_b = *col_blu_b;
rgba_t red_v = *col_red_v;
rgba_t blu_v = *col_blu_v;
rgba_t red_u = *col_red_u;
rgba_t blu_u = *col_blu_u;
rgba_t gui = *col_guicolor;
rgba_t target = *col_target;

static InitRoutine init([]() {
  col_red.installChangeCallback([](settings::VariableBase<rgba_t> &var, rgba_t after) { red = after; });
  col_blu.installChangeCallback([](settings::VariableBase<rgba_t> &var, rgba_t after) { blu = after; });
  col_red_b.installChangeCallback([](settings::VariableBase<rgba_t> &var, rgba_t after) { red_b = after; });
  col_blu_b.installChangeCallback([](settings::VariableBase<rgba_t> &var, rgba_t after) { blu_b = after; });
  col_red_v.installChangeCallback([](settings::VariableBase<rgba_t> &var, rgba_t after) { red_v = after; });
  col_blu_v.installChangeCallback([](settings::VariableBase<rgba_t> &var, rgba_t after) { blu_v = after; });
  col_red_u.installChangeCallback([](settings::VariableBase<rgba_t> &var, rgba_t after) { red_u = after; });
  col_blu_u.installChangeCallback([](settings::VariableBase<rgba_t> &var, rgba_t after) { blu_u = after; });
  col_guicolor.installChangeCallback([](settings::VariableBase<rgba_t> &var, rgba_t after) { gui = after; });
  col_target.installChangeCallback([](settings::VariableBase<rgba_t> &var, rgba_t after) { target = after; });
});
}

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
        if ((type >= ITEM_HEALTH_SMALL && type <= ITEM_HEALTH_LARGE) || type == ITEM_TF2C_PILL)
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

// Timescale determines how fast it changes
rgba_t colors::Fade(rgba_t color_a, rgba_t color_b, float time, float timescale)
{
    // Determine how much percent should be used from color_a, also remap sin to be 0.0f -> 1.0f
    float percentage_a = fabsf(sin(time * timescale));
    rgba_t new_color;
    new_color.r = (color_b.r - color_a.r) * percentage_a + color_a.r;
    new_color.g = (color_b.g - color_a.g) * percentage_a + color_a.g;
    new_color.b = (color_b.b - color_a.b) * percentage_a + color_a.b;
    new_color.a = (color_b.a - color_a.a) * percentage_a + color_a.a;
    return new_color;
}

rgba_t colors::RainbowCurrent()
{
    return colors::FromHSL(fabs(sin(g_GlobalVars->curtime / 2.0f)) * 360.0f, 0.85f, 0.9f);
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
