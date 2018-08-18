#include <glez/draw.hpp>
#include <settings/Bool.hpp>
#include "hacks/LightESP.hpp"

static settings::Bool enable{ "lightesp.enable", "false" };

namespace hacks::shared::lightesp
{

Vector hitp[32];
Vector minp[32];
Vector maxp[32];
bool drawEsp[32];

void run()
{
#if ENABLE_VISUALS
    if (!enable)
        return;
    for (int i = 1; i < g_IEngine->GetMaxClients(); i++)
    {
        if (g_pLocalPlayer->entity_idx == i)
            continue;
        CachedEntity *pEntity = ENTITY(i);
        if (CE_BAD(pEntity) || !pEntity->m_bAlivePlayer())
        {
            drawEsp[i] = false;
            continue;
        }
        if (pEntity->m_iTeam() == LOCAL_E->m_iTeam() &&
            playerlist::IsDefault(pEntity))
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
#endif
}
void draw()
{
#if ENABLE_VISUALS
    if (!enable)
        return;
    for (int i = 1; i < g_IEngine->GetMaxClients(); i++)
    {
        if (!drawEsp[i])
            continue;
        CachedEntity *pEntity = ENTITY(i);
        if (CE_BAD(pEntity) || !pEntity->m_bAlivePlayer())
            continue;
        if (pEntity == LOCAL_E)
            continue;
        Vector out;
        if (draw::WorldToScreen(hitp[i], out))
        {
            float size;
            Vector pout, pout2;
            if (draw::WorldToScreen(minp[i], pout) &&
                draw::WorldToScreen(maxp[i], pout2))
                size = fmaxf(fabsf(pout2.x - pout.x), fabsf(pout2.y - pout.y));

            glez::draw::rect(out.x, out.y, size / 4, size / 4,
                             hacks::shared::lightesp::LightESPColor(pEntity));
        }
    }
#endif
}

rgba_t LightESPColor(CachedEntity *ent)
{
    if (!playerlist::IsDefault(ent))
    {
        return playerlist::Color(ent);
    }
    return colors::green;
}
} // namespace hacks::shared::lightesp
