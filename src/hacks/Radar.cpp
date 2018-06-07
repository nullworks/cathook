/*
 * Radar.cpp
 *
 *  Created on: Mar 28, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "Radar.hpp"

#ifndef FEATURE_RADAR_DISABLED
#if ENABLE_VISUALS

namespace hacks
{
namespace tf
{
namespace radar
{

static CatVar size(CV_INT, "radar_size", "300", "Radar size",
                   "Defines radar size in pixels");
static CatVar zoom(CV_FLOAT, "radar_zoom", "20", "Radar zoom",
                   "Defines radar zoom (1px = Xhu)");
static CatVar healthbar(CV_SWITCH, "radar_health", "1", "Radar healthbar",
                        "Show radar healthbar");
static CatVar enemies_over_teammates(
    CV_SWITCH, "radar_enemies_top", "1", "Show enemies on top",
    "If true, radar will render enemies on top of teammates");
static CatVar icon_size(CV_INT, "radar_icon_size", "20", "Icon size",
                        "Defines radar icon size");
static CatVar radar_enabled(CV_SWITCH, "radar", "0", "Enable", "Enable Radar");
static CatVar radar_x(CV_INT, "radar_x", "100", "Radar X",
                      "Defines radar position (X)");
static CatVar radar_y(CV_INT, "radar_y", "100", "Radar Y",
                      "Defines radar position (Y)");
static CatVar
    use_icons(CV_SWITCH, "radar_icons", "1", "Use Icons",
              "Radar will use class icons instead of class portraits");
static CatVar show_teammates(CV_SWITCH, "radar_teammates", "1",
                             "Show Teammates");
static CatVar show_healthpacks(CV_SWITCH, "radar_healthpacks", "1",
                               "Show Healthpacks");
static CatVar show_ammopacks(CV_SWITCH, "radar_ammopacks", "1",
                             "Show Ammopacks");
Timer invalid{};

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

    if (nx < -halfsize)
        nx = -halfsize;
    if (nx > halfsize)
        nx = halfsize;
    if (ny < -halfsize)
        ny = -halfsize;
    if (ny > halfsize)
        ny = halfsize;

    return { nx + halfsize - (int) icon_size / 2,
             ny + halfsize - (int) icon_size / 2 };
}
bool loaded = false;

void DrawEntity(int x, int y, CachedEntity *ent)
{

    static textures::texture_atlas texture(DATA_PATH "/res/atlas.png", 1024,
                                           512);
    if (!loaded)
    {
        if (texture.texture.handle == GLEZ_TEXTURE_INVALID &&
            invalid.test_and_set(10000))
        {
            logging::Info("Invalid atlas, retrying....");
            texture.texture.handle =
                glez_texture_load_png_rgba(DATA_PATH "/res/atlas.png");
            return;
        }
        else if (texture.texture.handle != GLEZ_TEXTURE_INVALID)
            loaded = true;
        return;
    }
    struct basesprite
    {
        textures::sprite sprite = texture.create_sprite(0, 0, 0, 0);
    };

    static std::array<std::array<basesprite, 9>, 3> tx_class;
    static std::array<basesprite, 2> tx_teams;
    static std::array<basesprite, 2> tx_items;
    bool call = false;
    if (call)
        goto label1;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 9; j++)
            tx_class[i][j].sprite.setsprite(64 * j, 64 * i, 64, 64);
    tx_teams[0].sprite.setsprite(11 * 64, 128, 64, 64);
    tx_teams[1].sprite.setsprite(11 * 64, 64, 64, 64);
    tx_items[0].sprite.setsprite(10 * 64, 64, 64, 64);
    tx_items[1].sprite.setsprite(10 * 64, 128, 64, 64);
    call = true;
label1:
    int idx = -1;
    rgba_t clr;
    float healthp = 0.0f;

    if (CE_GOOD(ent))
    {
        if (ent->m_Type() == ENTITY_PLAYER)
        {
            if (CE_BYTE(ent, netvar.iLifeState))
                return; // DEAD. not big surprise.
            const int &clazz = CE_INT(ent, netvar.iClass);
            const int &team  = CE_INT(ent, netvar.iTeamNum);
            idx              = team - 2;
            if (idx < 0 || idx > 1)
                return;
            if (clazz <= 0 || clazz > 9)
                return;
            const auto &wtr =
                WorldToRadar(ent->m_vecOrigin().x, ent->m_vecOrigin().y);

            if (use_icons)
            {
                tx_teams[idx].sprite.draw(x + wtr.first, y + wtr.second,
                                          (int) icon_size, (int) icon_size,
                                          colors::white);
                tx_class[2][clazz - 1].sprite.draw(
                    x + wtr.first, y + wtr.second, (int) icon_size,
                    (int) icon_size, colors::white);
            }
            else
            {
                tx_class[idx][clazz - 1].sprite.draw(
                    x + wtr.first, y + wtr.second, (int) icon_size,
                    (int) icon_size, colors::white);
                draw_api::draw_rect_outlined(
                    x + wtr.first, y + wtr.second, (int) icon_size,
                    (int) icon_size, idx ? colors::blu_v : colors::red_v, 0.5f);
            }

            if (ent->m_iMaxHealth() && healthbar)
            {
                healthp =
                    (float) ent->m_iHealth() / (float) ent->m_iMaxHealth();
                clr = colors::Health(ent->m_iHealth(), ent->m_iMaxHealth());
                if (healthp > 1.0f)
                    healthp = 1.0f;
                draw_api::draw_rect_outlined(
                    x + wtr.first, y + wtr.second + (int) icon_size,
                    (int) icon_size, 4, colors::black, 0.5f);
                draw_api::draw_rect(
                    x + wtr.first + 1, y + wtr.second + (int) icon_size + 1,
                    ((float) icon_size - 2.0f) * healthp, 2, clr);
            }
        }
        else if (ent->m_Type() == ENTITY_BUILDING)
        {
            /*if (ent->m_iClassID() == CL_CLASS(CObjectDispenser)) {
                const int& team = CE_INT(ent, netvar.iTeamNum);
                int idx = team - 2;
                if (idx < 0 || idx > 1) return;
                const auto& wtr = WorldToRadar(ent->m_vecOrigin().x,
            ent->m_vecOrigin().y); buildings[0].Draw(x + wtr.first, y +
            wtr.second, (int)icon_size, (int)icon_size, idx ? colors::blu :
            colors::red	); draw::OutlineRect(x + wtr.first, y + wtr.second,
            (int)icon_size, (int)icon_size, idx ? colors::blu_v :
            colors::red_v); if (ent->m_iMaxHealth() && healthbar) { float
            healthp
            = (float)ent->m_iHealth() / (float)ent->m_iMaxHealth(); int clr =
            colors::Health(ent->m_iHealth(), ent->m_iMaxHealth()); if (healthp
            > 1.0f) healthp = 1.0f; draw::OutlineRect(x + wtr.first, y +
            wtr.second + (int)icon_size, (int)icon_size, 4, colors::black);
                    draw::DrawRect(x + wtr.first + 1, y + wtr.second +
            (int)icon_size + 1, ((float)icon_size - 2.0f) * healthp, 2, clr);
                }
            }*/
        }
        else if (ent->m_Type() == ENTITY_GENERIC)
        {
            if (show_healthpacks && (ent->m_ItemType() == ITEM_HEALTH_LARGE ||
                                     ent->m_ItemType() == ITEM_HEALTH_MEDIUM ||
                                     ent->m_ItemType() == ITEM_HEALTH_SMALL))
            {
                const auto &wtr =
                    WorldToRadar(ent->m_vecOrigin().x, ent->m_vecOrigin().y);
                float sz  = float(icon_size) * 0.15f * 0.5f;
                float sz2 = float(icon_size) * 0.85;
                tx_items[1].sprite.draw(x + wtr.first + sz, y + wtr.second + sz,
                                        sz2, sz2, colors::white);
            }
            else if (show_ammopacks && (ent->m_ItemType() == ITEM_AMMO_LARGE ||
                                        ent->m_ItemType() == ITEM_AMMO_MEDIUM ||
                                        ent->m_ItemType() == ITEM_AMMO_SMALL))
            {
                const auto &wtr =
                    WorldToRadar(ent->m_vecOrigin().x, ent->m_vecOrigin().y);
                float sz  = float(icon_size) * 0.15f * 0.5f;
                float sz2 = float(icon_size) * 0.85;
                tx_items[0].sprite.draw(x + wtr.first + sz, y + wtr.second + sz,
                                        sz2, sz2, colors::white);
            }
        }
    }
}

void Draw()
{
    if (!g_IEngine->IsInGame())
        return;
    if (CE_BAD(LOCAL_E))
        return;
    int x, y;
    rgba_t outlineclr;
    std::vector<CachedEntity *> enemies{};
    CachedEntity *ent;

    if (!radar_enabled)
        return;
    x              = (int) radar_x;
    y              = (int) radar_y;
    int radar_size = size;
    int half_size  = radar_size / 2;

    outlineclr = GUIColor();

    draw_api::draw_rect(x, y, radar_size, radar_size,
                        colors::Transparent(colors::black, 0.4f));
    draw_api::draw_rect_outlined(x, y, radar_size, radar_size, outlineclr,
                                 0.5f);

    if (enemies_over_teammates)
        enemies.clear();
    for (int i = 1; i < HIGHEST_ENTITY; i++)
    {
        ent = ENTITY(i);
        if (CE_BAD(ent))
            continue;
        if (!ent->m_bAlivePlayer())
            continue;
        if (i == g_IEngine->GetLocalPlayer())
            continue;
        if (!show_teammates && ent->m_Type() == ENTITY_PLAYER &&
            !ent->m_bEnemy())
            continue;
        if (!enemies_over_teammates || !show_teammates ||
            ent->m_Type() != ENTITY_PLAYER)
            DrawEntity(x, y, ent);
        else if (ent->m_bEnemy())
            enemies.push_back(ent);
        else
            DrawEntity(x, y, ent);
    }
    if (enemies_over_teammates && show_teammates)
        for (auto enemy : enemies)
            DrawEntity(x, y, enemy);
    if (CE_GOOD(LOCAL_E))
    {
        DrawEntity(x, y, LOCAL_E);
        const auto &wtr = WorldToRadar(g_pLocalPlayer->v_Origin.x,
                                       g_pLocalPlayer->v_Origin.y);
        if (!use_icons)
            draw_api::draw_rect_outlined(x + wtr.first, y + wtr.second,
                                         int(icon_size), int(icon_size),
                                         GUIColor(), 0.5f);
    }

    draw_api::draw_line(x + half_size, y + half_size / 2, 0, half_size,
                        colors::Transparent(GUIColor(), 0.4f), 0.5f);
    draw_api::draw_line(x + half_size / 2, y + half_size, half_size, 0,
                        colors::Transparent(GUIColor(), 0.4f), 0.5f);
}
}
}
}

#endif
#endif
