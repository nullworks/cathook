/*
 * Created on 29.07.18.
 */

#include "common.hpp"
#include "hacks/AntiAntiAim.hpp"
#include "sdk/dt_recv_redef.h"

namespace hacks::shared::anti_anti_aim
{
static settings::Boolean enable{ "anti-anti-aim.enable", "false" };
static settings::Boolean debug{ "anti-anti-aim.debug.enable", "false" };

std::unordered_map<unsigned, brutedata> resolver_map;
std::array<CachedEntity *, 32> sniperdot_array;

static inline void modifyAnlges()
{
    for (int i = 1; i <= g_IEngine->GetMaxClients(); i++)
    {
        auto player = ENTITY(i);
        if (CE_BAD(player) || !player->m_bAlivePlayer() || !player->m_bEnemy() || !player->player_info.friendsID)
            continue;
        auto &data  = resolver_map[player->player_info.friendsID];
        auto &angle = CE_VECTOR(player, netvar.m_angEyeAngles);
        angle.x     = data.new_angle.x;
        angle.y     = data.new_angle.y;
    }
}

static inline void resetangles()
{
    for (int i = 1; i <= g_IEngine->GetMaxClients(); i++)
    {
        auto player = ENTITY(i);
        if (CE_BAD(player) || !player->m_bAlivePlayer() || !player->m_bEnemy() || !player->player_info.friendsID)
            continue;
        auto &data  = resolver_map[player->player_info.friendsID];
        CE_VECTOR(player, netvar.m_angEyeAngles) = CE_VECTOR(player, netvar.m_angEyeAngles);
    }
}

static inline void CreateMove()
{
    // Empty the array
    sniperdot_array.fill(0);
    // Find sniper dots
    for (int i = g_IEngine->GetMaxClients() + 1; i <= HIGHEST_ENTITY; i++)
    {
        CachedEntity *dot_ent = ENTITY(i);
        // Not a sniper dot
        if (CE_BAD(dot_ent) || dot_ent->m_iClassID() != CL_CLASS(CSniperDot))
            continue;
        // Get the player it belongs to
        auto ent_idx = HandleToIDX(CE_INT(dot_ent, netvar.m_hOwnerEntity));
        // IDX check
        if (IDX_BAD(ent_idx) || ent_idx > sniperdot_array.size() || ent_idx <= 0)
            continue;
        // Good sniper dot, add to array
        sniperdot_array.at(ent_idx - 1) = dot_ent;
    }
}

void frameStageNotify(ClientFrameStage_t stage)
{
#if !ENABLE_TEXTMODE
    if (!g_IEngine->IsInGame())
        return;

    if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
    {
        if(!enable)
            resetangles();
        else
            modifyAnlges();
    }
#endif
}

static std::array<float, 2> spin_yaw_resolves{ 0.0f, 180.0f};
static std::array<float, 2> static_yaw_resolves{90.0f, -90.0f};
static std::array<std::string, 4> yaw_resolvetype{"NONE", "OPPOSITE", "LEFT", "RIGHT"};

static float resolveAngleYaw(float angle, brutedata &brute, CachedEntity* ent)
{
    brute.original_angle.y = angle;
    float delta;

    while (angle > 180)
        angle -= 360;

    while (angle < -180)
        angle += 360;
    Vector vecangle;
    // Yaw Resolving
    // Find out which angle we should try
    if(CE_GOOD(LOCAL_E))
        vecangle = GetAimAtAngles(LOCAL_E->m_vecOrigin(), ent->m_vecOrigin());

    delta = brute.previous_angle.y - angle;
    
    {
        int entry = (int) std::floor((brute.brutenum / 2.0f)) % static_yaw_resolves.size();
        vecangle.y += static_yaw_resolves[entry];
    }

    while (vecangle.y > 180)
        vecangle.y -= 360;

    while (vecangle.y < -180)
        vecangle.y += 360;

    brute.new_angle.y = vecangle.y;

    brute.previous_delta.y = delta;
    return vecangle.y;
}

static float resolveAnglePitch(float angle, brutedata &brute, CachedEntity *ent)
{
    brute.original_angle.x = angle;

    while (angle > 90)
        angle -= 90;

    while (angle < -90)
        angle += 90;

    // Get CSniperDot associated with entity
    CachedEntity *sniper_dot = nullptr;

    // Get Weapon id
    auto weapon_id = HandleToIDX(CE_INT(ent, netvar.hActiveWeapon));

    // Check IDX for validity
    if (IDX_GOOD(weapon_id))
    {
        auto weapon_ent = ENTITY(weapon_id);
        // Check weapon for validity
        if (CE_GOOD(weapon_ent) && (weapon_ent->m_iClassID() == CL_CLASS(CTFSniperRifle) || weapon_ent->m_iClassID() == CL_CLASS(CTFSniperRifleDecap) || weapon_ent->m_iClassID() == CL_CLASS(CTFSniperRifleClassic)))
        {
            // Get Sniperdot
            sniper_dot = sniperdot_array.at(ent->m_IDX - 1);
            // Check if the dot is still good, if not then set to nullptr
            if (CE_BAD(sniper_dot) || sniper_dot->m_iClassID() != CL_CLASS(CSniperDot))
                sniper_dot = nullptr;
        }
    }
    // No sniper dot/not using a sniperrifle.
    if (sniper_dot == nullptr)
    {

        if (brute.brutenum % 3)
        {
            // Pitch resolver
            if (angle >= 90)
                angle = -89;
            else if (angle <= -90)
                angle = 89;
            else
                angle = 0;
        }

        if(brute.previous_angle.x == -89 && brute.hits_in_a_row % 2 == 0)
            angle = 89;
        else
            angle = -89;
    }
    // Sniper dot found, use it.
    else
    {
        // Get End and start point
        auto dot_origin = sniper_dot->m_vecOrigin();
        auto eye_origin = re::C_BasePlayer::GetEyePosition(RAW_ENT(ent));
        // Get Angle from eye to dot
        Vector diff = dot_origin - eye_origin;
        Vector angles;
        VectorAngles(diff, angles);
        // Use the pitch (yaw is not useable because sadly the sniper dot does not represent it with fake yaw)
        angle = angles.x;
    }

    brute.new_angle.x = angle;
    brute.previous_angle.x = angle;
    return angle;
}

void increaseBruteNum(int idx)
{
    auto ent = ENTITY(idx);
    if (CE_BAD(ent) || !ent->player_info.friendsID)
        return;
    auto &data = hacks::shared::anti_anti_aim::resolver_map[ent->player_info.friendsID];
    if (data.hits_in_a_row >= 4)
        data.hits_in_a_row = 2;
    else if (data.hits_in_a_row >= 2)
        data.hits_in_a_row = 0;
    else
    {
        data.brutenum++;
        if (debug)
            logging::Info("AAA: Brutenum for entity %i increased to %i", idx, data.brutenum);
        data.hits_in_a_row = 0;
        auto &angle        = CE_VECTOR(ent, netvar.m_angEyeAngles);
        angle.x            = resolveAnglePitch(data.original_angle.x, data, ent);
        angle.y            = resolveAngleYaw(data.original_angle.y, data, ent);
        data.previous_angle.y = data.new_angle.x   = angle.x;
        data.previous_angle.y = data.new_angle.y   = angle.y;
    }
}

static void pitchHook(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
    float flPitch      = pData->m_Value.m_Float;
    float *flPitch_out = (float *) pOut;

    if (!enable)
    {
        *flPitch_out = flPitch;
        return;
    }

    auto client_ent   = (IClientEntity *) (pStruct);
    CachedEntity *ent = ENTITY(client_ent->entindex());
    if (CE_GOOD(ent))
        *flPitch_out = resolveAnglePitch(flPitch, resolver_map[ent->player_info.friendsID], ent);
}

static void yawHook(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
    float flYaw      = pData->m_Value.m_Float;
    float *flYaw_out = (float *) pOut;

    if (!enable)
    {
        *flYaw_out = flYaw;
        return;
    }

    auto client_ent   = (IClientEntity *) (pStruct);
    CachedEntity *ent = ENTITY(client_ent->entindex());
    if (CE_GOOD(ent))
        *flYaw_out = resolveAngleYaw(flYaw, resolver_map[ent->player_info.friendsID], ent);
}

// *_ptr points to what we need to modify while *_ProxyFn holds the old value
static RecvVarProxyFn *original_ptrX;
static RecvVarProxyFn original_ProxyFnX;
static RecvVarProxyFn *original_ptrY;
static RecvVarProxyFn original_ProxyFnY;

static void hook()
{
    auto pClass = g_IBaseClient->GetAllClasses();
    while (pClass)
    {
        const char *pszName = pClass->m_pRecvTable->m_pNetTableName;
        // "DT_TFPlayer", "tfnonlocaldata"
        if (!strcmp(pszName, "DT_TFPlayer"))
        {
            for (int i = 0; i < pClass->m_pRecvTable->m_nProps; i++)
            {
                RecvPropRedef *pProp1 = (RecvPropRedef *) &(pClass->m_pRecvTable->m_pProps[i]);
                if (!pProp1)
                    continue;
                const char *pszName2 = pProp1->m_pVarName;
                if (!strcmp(pszName2, "tfnonlocaldata"))
                    for (int j = 0; j < pProp1->m_pDataTable->m_nProps; j++)
                    {
                        RecvPropRedef *pProp2 = (RecvPropRedef *) &(pProp1->m_pDataTable->m_pProps[j]);
                        if (!pProp2)
                            continue;
                        const char *name = pProp2->m_pVarName;

                        // Pitch Fix
                        if (!strcmp(name, "m_angEyeAngles[0]"))
                        {
                            original_ptrX     = &pProp2->m_ProxyFn;
                            original_ProxyFnX = pProp2->m_ProxyFn;
                            pProp2->m_ProxyFn = pitchHook;
                        }

                        // Yaw Fix
                        if (!strcmp(name, "m_angEyeAngles[1]"))
                        {
                            original_ptrY = &pProp2->m_ProxyFn;
                            logging::Info("Yaw Fix Applied");
                            original_ProxyFnY = pProp2->m_ProxyFn;
                            pProp2->m_ProxyFn = yawHook;
                        }
                    }
            }
        }
        pClass = pClass->m_pNext;
    }
}

static void shutdown()
{
    *original_ptrX = original_ProxyFnX;
    *original_ptrY = original_ProxyFnY;
}

static InitRoutine init([]() {
    hook();
    EC::Register(EC::Shutdown, shutdown, "antiantiaim_shutdown");
    EC::Register(EC::CreateMove, CreateMove, "cm_antiantiaim");
#if ENABLE_TEXTMODE
    EC::Register(EC::CreateMove, modifyAnlges, "cm_textmodeantiantiaim");
#endif
});
} // namespace hacks::shared::anti_anti_aim