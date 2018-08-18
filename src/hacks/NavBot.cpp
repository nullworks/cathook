//
// Created by bencat07 on 17.08.18.
//
#include "common.hpp"
#include "navparser.hpp"
#include "NavBot.hpp"

namespace hacks::tf2::NavBot
{
static settings::Bool enable("navbot.enable", "false");
static settings::Bool spy_mode("navbot.spy-mode", "false");
static settings::Bool heavy_mode("navbot.heavy-mode", "false");
static settings::Bool primary_only("navbot.primary-only", "true");

bool HasLowAmmo()
{
    int *weapon_list =
        (int *) ((unsigned) (RAW_ENT(LOCAL_E)) + netvar.hMyWeapons);
    for (int i = 0; weapon_list[i]; i++)
    {
        int handle = weapon_list[i];
        int eid    = handle & 0xFFF;
        if (eid >= 32 && eid <= HIGHEST_ENTITY)
        {
            IClientEntity *weapon = g_IEntityList->GetClientEntity(eid);
            if (weapon and
                re::C_BaseCombatWeapon::IsBaseCombatWeapon(weapon) and
                re::C_TFWeaponBase::UsesPrimaryAmmo(weapon) and
                not re::C_TFWeaponBase::HasPrimaryAmmo(weapon))
                return true;
        }
    }
    return false;
}

bool HasLowHealth()
{
    return float(LOCAL_E->m_iHealth()) / float(LOCAL_E->m_iMaxHealth()) < 0.64;
}

CachedEntity *nearestHealth()
{
    float bestscr         = FLT_MAX;
    CachedEntity *bestent = nullptr;
    for (int i = 0; i < HIGHEST_ENTITY; i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_BAD(ent) || ent->m_iClassID() != CL_CLASS(CBaseAnimating))
            continue;
        if (ent->m_ItemType() != ITEM_HEALTH_SMALL &&
            ent->m_ItemType() != ITEM_HEALTH_MEDIUM &&
            ent->m_ItemType() != ITEM_HEALTH_LARGE)
            continue;
        if (ent->m_flDistance() < bestscr)
        {
            bestscr = ent->m_flDistance();
            bestent = ent;
        }
    }
    return bestent;
}
CachedEntity *nearestAmmo()
{
    float bestscr         = FLT_MAX;
    CachedEntity *bestent = nullptr;
    for (int i = 0; i < HIGHEST_ENTITY; i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_BAD(ent) || ent->m_iClassID() != CL_CLASS(CBaseAnimating))
            continue;
        if (ent->m_ItemType() != ITEM_AMMO_SMALL &&
            ent->m_ItemType() != ITEM_AMMO_MEDIUM &&
            ent->m_ItemType() != ITEM_AMMO_LARGE)
            continue;
        if (ent->m_flDistance() < bestscr)
        {
            bestscr = ent->m_flDistance();
            bestent = ent;
        }
    }
    return bestent;
}
CachedEntity *NearestEnemy()
{
    float bestscr         = FLT_MAX;
    CachedEntity *bestent = nullptr;
    for (int i = 0; i < g_IEngine->GetMaxClients(); i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_BAD(ent) || ent->m_Type() != ENTITY_PLAYER)
            continue;
        if (ent == LOCAL_E || !ent->m_bAlivePlayer() ||
            ent->m_iTeam() == LOCAL_E->m_iTeam())
            continue;
        float scr = ent->m_flDistance();
        if (g_pPlayerResource->GetClass(ent) == tf_engineer)
            scr *= 5.0f;
        if (g_pPlayerResource->GetClass(ent) == tf_pyro)
            scr *= 7.0f;
        if (scr < bestscr)
        {
            bestscr = scr;
            bestent = ent;
        }
    }
    return bestent;
}
std::vector<Vector> sniper_spots;
void Init()
{
    sniper_spots.clear();
    for (auto area : nav::areas)
        for (auto hide : area.m_hidingSpots)
            if (hide.IsGoodSniperSpot() || hide.IsIdealSniperSpot() ||
                hide.IsExposed())
                sniper_spots.push_back(hide.m_pos);
}

Timer slot_timer{};
void UpdateSlot()
{
    if (!slot_timer.test_and_set(1000))
        return;
    if (CE_GOOD(LOCAL_E) && CE_GOOD(LOCAL_W) && !g_pLocalPlayer->life_state)
    {
        IClientEntity *weapon = RAW_ENT(LOCAL_W);
        // IsBaseCombatWeapon()
        if (re::C_BaseCombatWeapon::IsBaseCombatWeapon(weapon))
        {
            int slot    = re::C_BaseCombatWeapon::GetSlot(weapon);
            int newslot = 1;
            if (spy_mode)
                newslot = 3;
            if (slot != newslot - 1)
                g_IEngine->ClientCmd_Unrestricted(
                    format("slot", newslot).c_str());
        }
    }
}
Timer cdr{};
Timer cd2{};
Timer cd3{};
void CreateMove()
{
    if (!enable || !nav::Prepare())
        return;
    if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer())
        return;
    if (primary_only)
        UpdateSlot();
    if (HasLowHealth() && cdr.test_and_set(5000))
    {
        CachedEntity *med = nearestHealth();
        if (CE_GOOD(med))
        {
            nav::NavTo(med->m_vecOrigin());
            return;
        }
    }
    if (HasLowAmmo() && cdr.test_and_set(5000))
    {
        CachedEntity *ammo = nearestAmmo();
        if (CE_GOOD(ammo))
        {
            nav::NavTo(ammo->m_vecOrigin());
            return;
        }
    }
    if (!nav::ReadyForCommands && !spy_mode && !heavy_mode)
        cd3.update();
    bool isready = (spy_mode || heavy_mode) ? 1 : nav::ReadyForCommands;
    int waittime = (spy_mode || heavy_mode) ? 100 : 5000;
    if (isready && cd3.test_and_set(waittime))
    {
        if (!spy_mode && !heavy_mode)
        {
            cd3.update();
            Vector random_spot;
            if (sniper_spots.empty())
            {
                if (cd2.test_and_set(5000))
                    Init();
                return;
            }
            int rng     = rand() % sniper_spots.size();
            random_spot = sniper_spots.at(rng);
            if (random_spot.z)
                nav::NavTo(random_spot, false, true);
        }
        else
        {
            CachedEntity *tar = NearestEnemy();
            if (CE_BAD(tar))
            {
                Vector random_spot;
                if (sniper_spots.empty())
                {
                    if (cd2.test_and_set(5000))
                        Init();
                    return;
                }
                int rng     = rand() % sniper_spots.size();
                random_spot = sniper_spots.at(rng);
                if (random_spot.z)
                    nav::NavTo(random_spot, false);
                return;
            }
            nav::NavTo(tar->m_vecOrigin(), false);
        }
    }
}
} // namespace hacks::tf2::NavBot
