#include "common.hpp"
#include "PlayerTools.hpp"
#include "playerresource.h"
#include "soundcache.hpp"

namespace hacks::shared::tracers
{

static settings::Boolean enabled("tracers.enabled", "false");
static settings::Boolean teammates("tracers.teammates", "false");
static settings::Int coloring_mode("tracers.coloring-mode", "0");
static settings::Float green_dist("tracers.green-distance", "1500");
static settings::Float max_dist("tracers.max-dist", "0");
static settings::Float min_fov("tracers.min-fov", "0");
static settings::Float line_thickness("tracers.line-thickness", "2");
static settings::Float opaque("tracers.line-opaqueness", "255");
static settings::Boolean buildings("tracers.buildings", "true");

// 0 = don't, 1 = yes but only in enemy team, 2 = always
static settings::Int draw_friendlies("tracers.draw-friends", "1");

// Extend a line to a certain border
// https://stackoverflow.com/a/45056039
static inline Vector2D toBorder(float x1, float y1, float x2, float y2, float left, float top, float right, float bottom)
{
    float dx, dy, py, vx, vy;
    vx = x2 - x1;
    vy = y2 - y1;
    dx = vx < 0 ? left : right;
    dy = py = vy < 0 ? top : bottom;
    if (vx == 0)
    {
        dx = x1;
    }
    else if (vy == 0)
    {
        dy = y1;
    }
    else
    {
        dy = y1 + (vy / vx) * (dx - x1);
        if (dy < top || dy > bottom)
        {
            dx = x1 + (vx / vy) * (py - y1);
            dy = py;
        }
    }
    return { dx, dy };
}
struct color_determine
{
    float pct;
    rgba_t color;
    color_determine(float _pct, rgba_t _color)
    {
        pct   = _pct;
        color = _color;
    }
};

inline std::optional<rgba_t> getColor(CachedEntity *ent)
{
    if (ent->m_Type() == ENTITY_BUILDING)
    {
        if (!ent->m_vecDormantOrigin())
            return std::nullopt;
        if (!ent->m_bEnemy() && !teammates)
            return std::nullopt;
        float dist = ent->m_vecDormantOrigin()->DistTo(LOCAL_E->m_vecOrigin());
        if (*max_dist && dist > *max_dist)
            return std::nullopt;
        if (GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye, *ent->m_vecDormantOrigin()) < *min_fov)
            return std::nullopt;
        if (*coloring_mode == 0)
        {
            float hf = float(std::min(dist, *green_dist)) / float(*green_dist);
            rgba_t color(0.0f, 2.0f * hf, 2.0f * (1.0f - hf));
            color.g = std::min(1.0f, color.g);
            color.b = std::min(1.0f, color.b);
            return color;
        }
        else if (*coloring_mode == 1)
        {
            return colors::EntityF(ent);
        }
    }
    else
    {

        auto state = playerlist::AccessData(ent->player_info->friendsID);
        if (state.state == playerlist::k_EState::DEFAULT)
        {
            if (!ent->m_vecDormantOrigin())
                return std::nullopt;
            if (!ent->m_bEnemy() && !teammates)
                return std::nullopt;
            float dist = ent->m_vecDormantOrigin()->DistTo(LOCAL_E->m_vecOrigin());
            if (*max_dist && dist > *max_dist)
                return std::nullopt;
            if (*coloring_mode == 0)
                return colors::Health(std::min(dist, *green_dist), *green_dist);
            else if (*coloring_mode == 1)
                return colors::EntityF(ent);
        }
        if (!player_tools::shouldTargetSteamId(ent->player_info->friendsID))
        {
            if (*draw_friendlies == 1)
            {
                if (ent->m_bEnemy())
                    return colors::white;
            }
            else if (*draw_friendlies == 2)
                return colors::white;
            return std::nullopt;
        }
        if (!ent->m_bEnemy())
            return std::nullopt;
        return playerlist::Color(ent->player_info->friendsID);
    }
}

void draw()
{
    if (!enabled || CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer())
        return;
    // Loop all players
    if (*buildings)
    {
        for (auto const &ent : entity_cache::valid_ents)
        {
            // Get and check player
            const uint16_t curr_idx = ent->m_IDX;
            Vector origin;
            std::optional<rgba_t> color = std::nullopt;
            if (CE_INVALID(ent))
            {
                if (curr_idx > g_IEngine->GetMaxClients() || !g_pPlayerResource->isAlive(curr_idx))
                    continue;
                if (g_pPlayerResource->GetTeam(curr_idx) == g_pLocalPlayer->team && !teammates)
                    continue;
                auto vec = soundcache::GetSoundLocation(curr_idx);
                if (!vec)
                    continue;
                if (*max_dist && vec->DistTo(g_pLocalPlayer->v_Origin) > *max_dist)
                    continue;
                origin = *vec;
                color  = colors::FromRGBA8(160, 160, 160, *opaque);
            }
            else
            {
                if ((!RAW_ENT(ent)->IsDormant() && !ent->m_bAlivePlayer()) || !ent->m_vecDormantOrigin())
                    continue;
                if (curr_idx <= g_IEngine->GetMaxClients() && !g_pPlayerResource->isAlive(curr_idx))
                    continue;
                origin = *ent->m_vecDormantOrigin();
                if (*buildings)
                    if (ent->m_Type() != ENTITY_PLAYER && ent->m_Type() != ENTITY_BUILDING)
                        continue;
                if (ent == LOCAL_E)
                    continue;
                color = getColor(ent);
                if (!color)
                    continue;
                if (RAW_ENT(ent)->IsDormant())
                    color = colors::FromRGBA8(160, 160, 160, *opaque);
                color->a = *opaque;
            }

            Vector out;
            if (!draw::WorldToScreen(origin, out))
            {
                // We need to flip on both x and y axis in case m_vecOrigin its not actually on screen
                out.x = draw::width - out.x;
                out.y = draw::height - out.y;

                auto extended = toBorder(draw::width / 2, draw::height / 2, out.x, out.y, 0, 0, draw::width, draw::height);
                out.x         = extended.x;
                out.y         = extended.y;
            }
            draw::Line(draw::width / 2, draw::height / 2, out.x - draw::width / 2, out.y - draw::height / 2, *color, *line_thickness);
        }
    }
    else
    {
        for (auto const &ent : entity_cache::player_cache)
        {
            // Get and check player

            Vector origin;
            int i                       = ent->m_IDX;
            std::optional<rgba_t> color = std::nullopt;

            if (CE_INVALID(ent))
            {
                if (i > g_IEngine->GetMaxClients() || !g_pPlayerResource->isAlive(i))
                    continue;
                if (g_pPlayerResource->GetTeam(i) == g_pLocalPlayer->team && !teammates)
                    continue;
                auto vec = soundcache::GetSoundLocation(i);
                if (!vec)
                    continue;
                if (*max_dist && vec->DistTo(g_pLocalPlayer->v_Origin) > *max_dist)
                    continue;
                origin = *vec;
                color  = colors::FromRGBA8(160, 160, 160, *opaque);
            }
            else
            {
                if ((!RAW_ENT(ent)->IsDormant() && !ent->m_bAlivePlayer()) || !ent->m_vecDormantOrigin())
                    continue;
                if (i <= g_IEngine->GetMaxClients() && !g_pPlayerResource->isAlive(i))
                    continue;
                origin = *ent->m_vecDormantOrigin();
                if (*buildings)
                    if (ent->m_Type() != ENTITY_PLAYER && ent->m_Type() != ENTITY_BUILDING)
                        continue;
                if (ent == LOCAL_E)
                    continue;
                color = getColor(ent);
                if (!color)
                    continue;
                if (RAW_ENT(ent)->IsDormant())
                    color = colors::FromRGBA8(160, 160, 160, *opaque);
                color->a = *opaque;
            }

            Vector out;
            if (!draw::WorldToScreen(origin, out))
            {
                // We need to flip on both x and y axis in case m_vecOrigin its not actually on screen
                out.x = draw::width - out.x;
                out.y = draw::height - out.y;

                auto extended = toBorder(draw::width / 2, draw::height / 2, out.x, out.y, 0, 0, draw::width, draw::height);
                out.x         = extended.x;
                out.y         = extended.y;
            }
            draw::Line(draw::width / 2, draw::height / 2, out.x - draw::width / 2, out.y - draw::height / 2, *color, *line_thickness);
        }
    }
}

static InitRoutine init([]() { EC::Register(EC::Draw, draw, "DRAW_tracers"); });
} // namespace hacks::shared::tracers
