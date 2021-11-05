/*
 * Ragdolls.cpp
 *
 *  Created on: Nov 6, 2019
 *      Author: Roboot
 */

#include "common.hpp"
#include "sdk/dt_recv_redef.h"
#include "Ragdolls.hpp"

namespace hacks::ragdolls
{

static settings::Int mode{ "visual.ragdoll-mode", "0" };
static settings::Boolean only_local{ "visual.ragdoll-only-local", "1" };

// Ragdoll override style
enum RagdollOverride_t
{
    NONE         = 0,
    GIB          = 1,
    BURNING      = 2,
    ELECTROCUTED = 3,
    ASH          = 4,
    GOLD         = 5,
    ICE          = 6
};

ProxyFnHook gib_hook;
ProxyFnHook burn_hook;
ProxyFnHook electro_hook;
ProxyFnHook ash_hook;
ProxyFnHook gold_hook;
ProxyFnHook ice_hook;

/**
 * Check to see if a ragdoll belongs to a player killed by the local player
 */
bool ragdollKillByLocal(void *ragdoll)
{
    // Get the owner of the ragdoll (TFPlayer)
    auto owner = g_IEntityList->GetClientEntity(NET_INT(ragdoll, netvar.m_iPlayerIndex));
    if (!owner || owner->IsDormant())
    {
        return false;
    }
    // Make sure this isn't the own player's ragdoll
    // The player will spectate iteself when the player dies via suicide
    if (owner->entindex() == g_pLocalPlayer->entity_idx)
    {
        return false;
    }
    // Check to see if the owner is spectating the local player
    auto owner_observer = g_IEntityList->GetClientEntityFromHandle(NET_VAR(owner, netvar.hObserverTarget, CBaseHandle));
    if (!owner_observer || owner_observer->IsDormant())
    {
        return false;
    }
    return owner_observer->entindex() == g_pLocalPlayer->entity_idx;
}

/**
 * Called for m_bGib
 */
void overrideGib(const CRecvProxyData *data, void *structure, void *out)
{
    auto gib = reinterpret_cast<bool *>(out);
    if (*mode == RagdollOverride_t::GIB && (!*only_local || ragdollKillByLocal(structure)))
        *gib = true;
    else
        *gib = data->m_Value.m_Int;
}

/**
 * Called for m_bBurning
 */
void overrideBurning(const CRecvProxyData *data, void *structure, void *out)
{
    auto burning = reinterpret_cast<bool *>(out);
    if (*mode == RagdollOverride_t::BURNING && (!*only_local || ragdollKillByLocal(structure)))
        *burning = true;
    else
        *burning = data->m_Value.m_Int;
}

/**
 * Called for m_bElectrocuted
 */
void overrideElectrocuted(const CRecvProxyData *data, void *structure, void *out)
{
    auto electrocuted = reinterpret_cast<bool *>(out);
    if (*mode == RagdollOverride_t::ELECTROCUTED && (!*only_local || ragdollKillByLocal(structure)))
        *electrocuted = true;
    else
        *electrocuted = data->m_Value.m_Int;
}

/**
 * Called for m_bBecomeAsh
 */
void overrideAsh(const CRecvProxyData *data, void *structure, void *out)
{
    auto ash = reinterpret_cast<bool *>(out);
    if (*mode == RagdollOverride_t::ASH && (!*only_local || ragdollKillByLocal(structure)))
        *ash = true;
    else
        *ash = data->m_Value.m_Int;
}

/**
 * Called for m_bGoldRagdoll
 */
void overrideGold(const CRecvProxyData *data, void *structure, void *out)
{
    auto gold = reinterpret_cast<bool *>(out);
    if (*mode == RagdollOverride_t::GOLD && (!*only_local || ragdollKillByLocal(structure)))
        *gold = true;
    else
        *gold = data->m_Value.m_Int;
}

/**
 * Called for m_bIceRagdoll
 */
void overrideIce(const CRecvProxyData *data, void *structure, void *out)
{
    auto ice = reinterpret_cast<bool *>(out);
    if (*mode == RagdollOverride_t::ICE && (!*only_local || ragdollKillByLocal(structure)))
        *ice = true;
    else
        *ice = data->m_Value.m_Int;
}

/**
 * Swap out the RecvVarProxyFns for TFRagdoll style props
 */
void hook()
{
    for (auto dt_class = g_IBaseClient->GetAllClasses(); dt_class; dt_class = dt_class->m_pNext)
    {
        auto table = dt_class->m_pRecvTable;

        if (strcmp(table->m_pNetTableName, "DT_TFRagdoll") == 0)
        {
            for (int i = 0; i < table->m_nProps; ++i)
            {
                auto prop = reinterpret_cast<RecvPropRedef *>(&table->m_pProps[i]);
                if (prop == nullptr)
                    continue;

                auto prop_name = prop->m_pVarName;
                if (strcmp(prop_name, "m_bGib") == 0)
                {
                    gib_hook.init(prop);
                    gib_hook.setHook(overrideGib);
                }
                else if (strcmp(prop_name, "m_bBurning") == 0)
                {
                    burn_hook.init(prop);
                    burn_hook.setHook(overrideBurning);
                }
                else if (strcmp(prop_name, "m_bElectrocuted") == 0)
                {
                    electro_hook.init(prop);
                    electro_hook.setHook(overrideElectrocuted);
                }
                else if (strcmp(prop_name, "m_bBecomeAsh") == 0)
                {
                    ash_hook.init(prop);
                    ash_hook.setHook(overrideAsh);
                }
                else if (strcmp(prop_name, "m_bGoldRagdoll") == 0)
                {
                    gold_hook.init(prop);
                    gold_hook.setHook(overrideGold);
                }
                else if (strcmp(prop_name, "m_bIceRagdoll") == 0)
                {
                    ice_hook.init(prop);
                    ice_hook.setHook(overrideIce);
                }
            }
        }
    }
}

/**
 * Restore the RecvVarProxyFns that were swapped out earlier
 */
void unhook()
{
    gib_hook.restore();
    burn_hook.restore();
    electro_hook.restore();
    ash_hook.restore();
    gold_hook.restore();
    ice_hook.restore();
}

static InitRoutine init(
    []()
    {
        hook();
        EC::Register(EC::Shutdown, unhook, "ragdoll_shutdown");
    });

} // namespace hacks::ragdolls
