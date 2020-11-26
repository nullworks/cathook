/*
 * Radar.cpp
 *
 *  Created on: Mar 28, 2017
 *      Author: nullifiedcat
 */
#include "common.hpp"
#include <settings/Int.hpp>

#ifndef FEATURE_RADAR_DISABLED
#if ENABLE_VISUALS

namespace hacks::tf::radar
{
static settings::Boolean radar_enabled{ "radar.enable", "false" };
static settings::Int shape{ "radar.shape", "0" };
static settings::Int size{ "radar.size", "300" };
static settings::Float zoom{ "radar.zoom", "10" };
static settings::Float opacity{ "radar.opacity", "0.4" };
static settings::Boolean show_cross{ "radar.show-cross", "true" };
static settings::Int healthbar{ "radar.healthbar", "2" };
static settings::Boolean enemies_over_teammates{ "radar.enemies-over-teammates", "true" };
static settings::Int icon_size{ "radar.icon-size", "20" };
static settings::Int radar_x{ "radar.x", "100" };
static settings::Int radar_y{ "radar.y", "100" };
static settings::Boolean use_icons{ "radar.use-icons", "true" };
static settings::Boolean show_teammates{ "radar.show.teammates", "true" };
static settings::Boolean show_teambuildings{ "radar.show.team.buildings", "true" };
static settings::Boolean show_healthpacks{ "radar.show.health", "true" };
static settings::Boolean show_ammopacks{ "radar.show.ammo", "true" };
static settings::Boolean hide_invis{ "radar.hide-invis", "false" };

static Timer invalid{};

std::pair<int, int> WorldToRadar(int x, int y)
{
    static int dx, dy, halfsize;
    static float ry, nx, ny;
    static QAngle angle;

    if (!zoom)
        return { 0, 0 };
    dx = x - g_pLocalPlayer->v_Origin.x;
    dy = y - g_pLocalPlayer->v_Origin.y;

    dx /= (float) zoom;
    dy /= (float) zoom;

    g_IEngine->GetViewAngles(angle);
    ry = DEG2RAD(angle.y) + PI / 2;

    dx = -dx;

    nx = dx * std::cos(ry) - dy * std::sin(ry);
    ny = dx * std::sin(ry) + dy * std::cos(ry);

    halfsize = (int) size / 2;

    if (*shape == 0)
    {
        if (nx < -halfsize)
            nx = -halfsize;
        if (nx > halfsize)
            nx = halfsize;
        if (ny < -halfsize)
            ny = -halfsize;
        if (ny > halfsize)
            ny = halfsize;

        return { nx + halfsize - (int) icon_size / 2, ny + halfsize - (int) icon_size / 2 };
    }

    else if (*shape == 1)
    {
        float PI = 3.14159265;

        float theta = atan2(ny, nx);

        if (nx > halfsize * std::cos(theta) && nx > 0)
            nx = halfsize * std::cos(theta);
        if (nx < halfsize * std::cos(theta) && nx < 0)
            nx = halfsize * std::cos(theta);
        if (ny > halfsize * std::sin(theta) && ny > 0)
            ny = halfsize * std::sin(theta);
        if (ny < halfsize * std::sin(theta) && ny < 0)
            ny = halfsize * std::sin(theta);

        return { nx + halfsize - (int) icon_size / 2, ny + halfsize - (int) icon_size / 2 };
    }
}
bool loaded = false;

static std::vector<std::vector<textures::sprite>> tx_class{};
static std::vector<textures::sprite> tx_teams{};
static std::vector<textures::sprite> tx_items{};
static std::vector<textures::sprite> tx_buildings{};
static std::vector<textures::sprite> tx_sentry{};

void DrawEntity(int x, int y, CachedEntity *ent)
{
    int idx = -1;
    rgba_t clr;
    float healthp = 0.0f;

    if (CE_VALID(ent))
    {
        if (ent->m_Type() == ENTITY_PLAYER)
        {
            if (!ent->m_bAlivePlayer())
                return; // DEAD. not big surprise.
            if (hide_invis && IsPlayerInvisible(ent))
                return;
            const int &clazz = CE_INT(ent, netvar.iClass);
            const int &team  = CE_INT(ent, netvar.iTeamNum);
            idx              = team - 2;
            if (idx < 0 || idx > 1)
                return;
            if (clazz <= 0 || clazz > 9)
                return;
            if (!ent->m_vecDormantOrigin())
                return;
            const auto &wtr = WorldToRadar(ent->m_vecDormantOrigin()->x, ent->m_vecDormantOrigin()->y);

            if (use_icons)
            {
                tx_teams[idx].draw(x + wtr.first, y + wtr.second, (int) icon_size, (int) icon_size, colors::white);
                tx_class[0][clazz - 1].draw(x + wtr.first, y + wtr.second, (int) icon_size, (int) icon_size, colors::white);
            }
            else
            {
                tx_class[2 - idx][clazz - 1].draw(x + wtr.first, y + wtr.second, (int) icon_size, (int) icon_size, colors::white);
                draw::RectangleOutlined(x + wtr.first, y + wtr.second, (int) icon_size, (int) icon_size, idx ? colors::blu_v : colors::red_v, 1.0f);
            }

            if (ent->m_iMaxHealth() && *healthbar > 0)
            {
                healthp = (float) ent->m_iHealth() / (float) ent->m_iMaxHealth();
                clr     = colors::Health(ent->m_iHealth(), ent->m_iMaxHealth());
                if (healthp > 1.0f)
                    healthp = 1.0f;
                draw::RectangleOutlined(x + wtr.first, y + wtr.second + (int) icon_size, (int) icon_size, 4, colors::black, 0.5f);
                draw::Rectangle(x + wtr.first + 1, y + wtr.second + (int) icon_size + 1, (*icon_size - 2.0f) * healthp, *healthbar, clr);
            }
        }
        else if (ent->m_Type() == ENTITY_BUILDING)
        {
            if (ent->m_iClassID() == CL_CLASS(CObjectDispenser) || ent->m_iClassID() == CL_CLASS(CObjectSentrygun) || ent->m_iClassID() == CL_CLASS(CObjectTeleporter))
            {
                if (!ent->m_vecDormantOrigin())
                    return;
                const auto &wtr = WorldToRadar(ent->m_vecDormantOrigin()->x, ent->m_vecDormantOrigin()->y);
                tx_teams[CE_INT(ent, netvar.iTeamNum) - 2].draw(x + wtr.first, y + wtr.second, *icon_size * 1.5f, *icon_size * 1.5f, colors::white);
                switch (ent->m_iClassID())
                {
                case CL_CLASS(CObjectDispenser):
                {
                    tx_buildings[0].draw(x + wtr.first, y + wtr.second, *icon_size * 1.5f, *icon_size * 1.5f, colors::white);
                    break;
                }
                case CL_CLASS(CObjectSentrygun):
                {
                    int level   = CE_INT(ent, netvar.iUpgradeLevel);
                    bool IsMini = CE_BYTE(ent, netvar.m_bMiniBuilding);
                    if (IsMini)
                        level = 4;
                    tx_sentry[level - 1].draw(x + wtr.first, y + wtr.second, *icon_size * 1.5f, *icon_size * 1.5f, colors::white);
                    break;
                }
                case CL_CLASS(CObjectTeleporter):
                {
                    tx_buildings[1].draw(x + wtr.first, y + wtr.second, *icon_size * 1.5f, *icon_size * 1.5f, colors::white);
                    break;
                }
                }
                if (ent->m_iMaxHealth() && *healthbar > 0)
                {
                    healthp = (float) ent->m_iHealth() / (float) ent->m_iMaxHealth();
                    clr     = colors::Health(ent->m_iHealth(), ent->m_iMaxHealth());
                    if (healthp > 1.0f)
                        healthp = 1.0f;
                    draw::RectangleOutlined(x + wtr.first, y + wtr.second + (int) icon_size * 1.5f, (int) icon_size * 1.5f, 4, colors::black, 0.5f);
                    draw::Rectangle(x + wtr.first + 1, y + wtr.second + ((int) icon_size + 1) * 1.5f, (*icon_size * 1.5f - 2.0f) * healthp, *healthbar, clr);
                }
            }
        }
        else if (ent->m_Type() == ENTITY_GENERIC)
        {
            if (!ent->m_vecDormantOrigin())
                return;
            if (show_healthpacks && (ent->m_ItemType() == ITEM_HEALTH_LARGE || ent->m_ItemType() == ITEM_HEALTH_MEDIUM || ent->m_ItemType() == ITEM_HEALTH_SMALL))
            {
                const auto &wtr = WorldToRadar(ent->m_vecDormantOrigin()->x, ent->m_vecDormantOrigin()->y);
                float sz        = *icon_size * 0.15f * 0.5f;
                float sz2       = *icon_size * 0.85;
                tx_items[0].draw(x + wtr.first + sz, y + wtr.second + sz, sz2, sz2, colors::white);
            }
            else if (show_ammopacks && (ent->m_ItemType() == ITEM_AMMO_LARGE || ent->m_ItemType() == ITEM_AMMO_MEDIUM || ent->m_ItemType() == ITEM_AMMO_SMALL))
            {
                const auto &wtr = WorldToRadar(ent->m_vecDormantOrigin()->x, ent->m_vecDormantOrigin()->y);
                float sz        = *icon_size * 0.15f * 0.5f;
                float sz2       = *icon_size * 0.85;
                tx_items[1].draw(x + wtr.first + sz, y + wtr.second + sz, sz2, sz2, colors::white);
            }
        }
    }
}

void Draw()
{
    if (!radar_enabled)
        return;
    if (!g_IEngine->IsInGame())
        return;
    if (CE_BAD(LOCAL_E))
        return;
    int x, y;
    rgba_t outlineclr;
    std::vector<CachedEntity *> enemies{};
    CachedEntity *ent;

    x              = (int) radar_x;
    y              = (int) radar_y;
    int radar_size = *size;
    int half_size  = radar_size / 2;

    outlineclr = GUIColor();

    if (*shape == 0)
    {
        draw::Rectangle(x, y, radar_size, radar_size, colors::Transparent(colors::black, *opacity));
        draw::RectangleOutlined(x, y, radar_size, radar_size, outlineclr, 0.5f);
    }
    else if (*shape == 1)
    {
        int center_x = x + half_size;
        int center_y = y + half_size;
        draw::Circle(center_x, center_y, half_size, outlineclr, 1, 100);
        draw::Circle(center_x, center_y, half_size / 2, colors::Transparent(colors::black, *opacity), half_size, 100);
    }

    if (enemies_over_teammates)
        enemies.clear();
    std::vector<CachedEntity *> sentries;
    for (int i = 1; i <= HIGHEST_ENTITY; i++)
    {
        ent = ENTITY(i);
        if (CE_INVALID(ent))
            continue;
        if (!ent->m_bAlivePlayer())
            continue;
        if (i == g_IEngine->GetLocalPlayer())
            continue;
        if (!show_teammates && ent->m_Type() == ENTITY_PLAYER && !ent->m_bEnemy())
            continue;
        if (!show_teambuildings && ent->m_Type() == ENTITY_BUILDING && !ent->m_bEnemy())
            continue;
        if (ent->m_iClassID() == CL_CLASS(CObjectSentrygun))
            sentries.push_back(ent);
        else if (!enemies_over_teammates || !show_teammates || ent->m_Type() != ENTITY_PLAYER)
            DrawEntity(x, y, ent);
        else if (ent->m_bEnemy())
            enemies.push_back(ent);
        else
            DrawEntity(x, y, ent);
    }
    if (enemies_over_teammates && show_teammates)
        for (auto enemy : enemies)
            DrawEntity(x, y, enemy);
    for (auto Sentry : sentries)
        DrawEntity(x, y, Sentry);
    if (CE_GOOD(LOCAL_E))
    {
        DrawEntity(x, y, LOCAL_E);
        const auto &wtr = WorldToRadar(g_pLocalPlayer->v_Origin.x, g_pLocalPlayer->v_Origin.y);
        if (!use_icons)
            draw::RectangleOutlined(x + wtr.first, y + wtr.second, int(icon_size), int(icon_size), GUIColor(), 0.5f);
    }

    if (show_cross)
    {
        draw::Line(x + half_size, y + half_size / 2, 0, half_size, colors::Transparent(GUIColor(), 0.4f), 0.5f);
        draw::Line(x + half_size / 2, y + half_size, half_size, 0, colors::Transparent(GUIColor(), 0.4f), 0.5f);
    }
}

static InitRoutine init([]() {
    // Background circles
    for (int i = 0; i < 2; ++i)
        tx_teams.push_back(textures::atlas().create_sprite(704, 384 + i * 64, 64, 64));
    // Items
    for (int i = 0; i < 2; ++i)
        tx_items.push_back(textures::atlas().create_sprite(640, 384 + i * 64, 64, 64));
    // Classes
    for (int i = 0; i < 3; ++i)
    {
        tx_class.emplace_back();
        for (int j = 0; j < 9; ++j)
            tx_class[i].push_back(textures::atlas().create_sprite(j * 64, 320 + i * 64, 64, 64));
    }
    for (int i = 0; i < 2; i++)
        tx_buildings.push_back(textures::atlas().create_sprite(576 + i * 64, 320, 64, 64));
    for (int i = 0; i < 4; i++)
        tx_sentry.push_back(textures::atlas().create_sprite(640 + i * 64, 256, 64, 64));
    logging::Info("Radar sprites loaded");
    EC::Register(EC::Draw, Draw, "radar", EC::average);
});
} // namespace hacks::tf::radar

#endif
#endif
