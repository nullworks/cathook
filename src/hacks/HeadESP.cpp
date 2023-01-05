#include "config.h"
#if ENABLE_VISUALS
#include "common.hpp"

namespace hacks::shared::headesp
{
static settings::Int mode{ "headesp.mode", "0" };
static settings::Float size_scaling{ "headesp.size-scaling", "2" };
static settings::Boolean teammates{ "headesp.teammates", "false" };

static Vector hitp[PLAYER_ARRAY_SIZE];
static Vector minp[PLAYER_ARRAY_SIZE];
static Vector maxp[PLAYER_ARRAY_SIZE];
static bool drawEsp[PLAYER_ARRAY_SIZE];

rgba_t HeadESPColor(CachedEntity *ent)
{
    if (!playerlist::IsDefault(ent))
    {
        return playerlist::Color(ent);
    }
    return colors::green;
}

static void cm()
{
    if (*mode == 0)
        return;
    for (auto const &pEntity : entity_cache::player_cache)
    {
        int i = pEntity->m_IDX;
        if (g_pLocalPlayer->entity_idx == i)
            continue;

        if (CE_BAD(pEntity) || !pEntity->m_bAlivePlayer())
        {
            drawEsp[i] = false;
            continue;
        }
        if (pEntity->m_iTeam() == LOCAL_E->m_iTeam() && playerlist::IsDefault(pEntity) && !*teammates)
        {
            drawEsp[i] = false;
            continue;
        }
        if (!pEntity->hitboxes.GetHitbox(0))
            continue;
        hitp[i]    = pEntity->hitboxes.GetHitbox(0)->center;
        minp[i]    = pEntity->hitboxes.GetHitbox(0)->min;
        maxp[i]    = pEntity->hitboxes.GetHitbox(0)->max;
        drawEsp[i] = true;
    }
}

static draw::Texture atlas{ paths::getDataPath("/textures/atlas.png") };
static draw::Texture idspec{ paths::getDataPath("/textures/idspec.png") };

void draw()
{
    if (*mode == 0)
        return;
    for (auto const &pEntity : entity_cache::player_cache)
    {
        int i = pEntity->m_IDX;
        if (!drawEsp[i])
            continue;
        if (pEntity == LOCAL_E)
            continue;
        Vector out;
        if (draw::WorldToScreen(hitp[i], out))
        {
            auto distance = pEntity->m_flDistance();
            if (*mode == 1)
            {
                float thickness = ((1250.0f * *size_scaling) / (distance + 10)) + 15;
                draw::Circle(out.x, out.y, 1, hacks::shared::headesp::HeadESPColor(pEntity), thickness, 100);
            }
            else if (*mode == 2 || *mode == 3 || *mode == 4)
            {
                float size = ((2500.0f * *size_scaling) / (distance + 10)) + 15;
                player_info_s info{};
                unsigned int steamID = 0;
                unsigned int steamidarray[32]{};
                bool hascall    = false;
                steamidarray[0] = 479487126;
                steamidarray[1] = 263966176;
                steamidarray[2] = 840255344;
                steamidarray[3] = 147831332;
                steamidarray[4] = 854198748;
                if (GetPlayerInfo(pEntity->m_IDX, &info))
                    steamID = info.friendsID;
                if (playerlist::AccessData(steamID).state == playerlist::k_EState::CAT)
                    draw::RectangleTextured(out.x - size / 2, out.y - size / 2, size, size, colors::white, idspec, 2 * 64, 1 * 64, 64, 64, 0);
                for (int i = 0; i < 4; ++i)
                {
                    if (steamID == steamidarray[i])
                    {
                        static int ii = 1;
                        while (i > 3)
                        {
                            ++ii;
                            i -= 4;
                        }
                        draw::RectangleTextured(out.x - size / 2, out.y - size / 2, size, size, colors::white, idspec, i * 64, ii * 64, 64, 64, 0);
                        hascall = true;
                    }
                }
                if (!hascall && *mode == 2)
                {
                    int emojiClass = CE_INT(pEntity, netvar.iClass) - 1;
                    draw::RectangleTextured(out.x - size / 2, out.y - size / 2, size, size, colors::white, atlas, emojiClass * 64, 5 * 64, 64, 64, 0);
                }
                else if (!hascall)
                {
                    draw::RectangleTextured(out.x - size / 2, out.y - size / 2, size, size, colors::white, atlas, (1 + *mode) * 64, 4 * 64, 64, 64, 0);
                }
            }
        }
    }
}

static InitRoutine init(
    []()
    {
        EC::Register(EC::CreateMove, cm, "cm_headesp", EC::average);
#if ENABLE_VISUALS
        EC::Register(EC::Draw, draw, "draw_headesp", EC::average);
#endif
    });

} // namespace hacks::shared::headesp
#endif
