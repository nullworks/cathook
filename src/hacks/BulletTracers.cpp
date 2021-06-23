/*
 * Credits  To Unknown For most of this
 */
#include "common.hpp"

namespace hacks::tf2::bullettracers
{

// CatEnum bullet_tracers_enum({ "OFF", "SNIPER RIFLES", "ALL WEAPONS", "LOCAL: SNIPER RIFLES", "LOCAL: ALL WEAPONS" });

static settings::Boolean enable{ "visual.bullet-tracers.enable", "false" };
static settings::Boolean sniper_only{ "visual.bullet-tracers.sniper-only", "false" };
static settings::Boolean local_only{ "visual.bullet-tracers.local-only", "false" };
static settings::Boolean draw_local{ "visual.bullet-tracers.draw-local", "true" };
static settings::Int team_mode{ "visual.bullet-tracers.teammode", "0" };
static settings::Boolean one_trace_only{ "visual.bullet-tracers.one-trace-only", "false" };
static settings::Boolean sentry_tracers{ "visual.bullet-tracers.sentry", "true" };

class CEffectData
{
public:
    Vector m_vStart{ 0 };           // 0
    Vector m_vEnd{ 0 };             // 12
    Vector m_vNormal{ 0 };          // 24
    QAngle m_vAngles{ 0, 0, 0 };    // 36
    int m_fFlags{ 0 };              // 48
    CBaseHandle m_hEntity{ -1, 0 }; // 52
    char pad0[28]{ 0 };             // 56
    int m_iEffectId{ 0 };           // 84
    char pad1[44]{ 0 };             // do NOT remove
};

typedef IClientEntity *(*GetActiveTFWeapon_t)(IClientEntity *);
typedef const char *(*GetParticleSystemNameFromIndex_t)(int);
typedef void (*DispatchEffect_t)(const char *, const CEffectData &);
typedef void (*GetAttachment_t)(IClientEntity *, int, Vector &);
typedef void (*CalcZoomedMuzzleLocation_t)(void *, Vector *, Vector *);
typedef int (*LookupAttachment_t)(IClientEntity *, const char *);

GetParticleSystemNameFromIndex_t GetParticleSystemNameFromIndex_fn;
GetActiveTFWeapon_t GetActiveTFWeapon_fn;
DispatchEffect_t DispatchEffect_fn;
CalcZoomedMuzzleLocation_t CalcZoomedMuzzleLocation_fn;

const char *AppropiateBeam(int team)
{
    if (team == TEAM_RED)
        return "dxhr_sniper_rail_red";
    else
        return "dxhr_sniper_rail_blue";
}

const char *GetParticleSystemNameFromIndex__detour(CEffectData &data)
{

    auto player       = g_IEntityList->GetClientEntityFromHandle(data.m_hEntity);
    auto wantedEffect = GetParticleSystemNameFromIndex_fn(data.m_iEffectId);
    if (!enable || !player || player->entindex() == -1)
        return wantedEffect;

    if (!strstr(wantedEffect, "bullet_") && data.m_iEffectId != 0xDEADCA7)
        return wantedEffect;

    // that player is a spy!
    if (*sentry_tracers && player->GetClientClass()->m_ClassID == CL_CLASS(CObjectSentrygun))
        return AppropiateBeam(NET_INT(player, netvar.iTeamNum));

    auto weapon = ENTITY(HandleToIDX(NET_INT(player, netvar.hActiveWeapon)));

    if (!weapon || weapon->m_IDX == 0)
        return wantedEffect;

    bool isLocal = player->entindex() == g_pLocalPlayer->entity_idx;

    if (sniper_only)
        if (weapon->m_iClassID() == CL_CLASS(CTFSniperRifle) || weapon->m_iClassID() == CL_CLASS(CTFSniperRifleDecap))
            return wantedEffect;
    if (!draw_local)
        if (isLocal)
            return wantedEffect;
    if (local_only)
        if (!isLocal)
            return wantedEffect;
    if (one_trace_only && data.m_iEffectId != 0xDEADCA7)
    {
        data.m_fFlags  = 1;
        data.m_hEntity = -1;
        return wantedEffect;
    }

    int team = NET_INT(player, netvar.iTeamNum);

    if (!isLocal)
        switch (*team_mode)
        {
        case 1:
            if (team == LOCAL_E->m_iTeam())
                return wantedEffect;
            break;
        case 2:
            if (team != LOCAL_E->m_iTeam())
                return wantedEffect;
            break;
        }
    return AppropiateBeam(team);
}

IClientEntity *GetActiveTFWeapon_detour(IClientEntity *this_ /* C_TFPlayer * */)
{
    auto weapon = GetActiveTFWeapon_fn(this_);
    if (!weapon || !enable)
        return weapon;

    if (CE_BAD(LOCAL_E) || IDX_BAD(this_->entindex()) || CE_BAD(ENTITY(this_->entindex())))
        return weapon;

    bool isLocal = this_->entindex() == g_pLocalPlayer->entity_idx;

    if (IDX_BAD(weapon->entindex()))
        return weapon;

    auto cweapon = ENTITY(weapon->entindex());
    if (CE_BAD(cweapon))
        return weapon;

    if (sniper_only)
        if (cweapon->m_iClassID() == CL_CLASS(CTFSniperRifle) || cweapon->m_iClassID() == CL_CLASS(CTFSniperRifleDecap))
            return weapon;
    if (local_only)
        if (!isLocal)
            return weapon;
    if (!draw_local)
        if (isLocal)
            return weapon;

    int team = NET_INT(this_, netvar.iTeamNum);

    if (!isLocal)
        switch (*team_mode)
        {
        case 1:
            if (team == LOCAL_E->m_iTeam())
                return weapon;
            break;
        case 2:
            if (team != LOCAL_E->m_iTeam())
                return weapon;
            break;
        }

    CEffectData data;

    data.m_hEntity = this_->GetRefEHandle();

    int attachment = vfunc<LookupAttachment_t>(weapon, 111, 0)(weapon, "muzzle");
    vfunc<GetAttachment_t>(weapon, 113, 0)(weapon, attachment, data.m_vStart);

    if (this_->entindex() == g_pLocalPlayer->entity_idx && g_pLocalPlayer->bZoomed)
        CalcZoomedMuzzleLocation_fn(this_, &g_pLocalPlayer->v_Eye, &data.m_vStart);

    {
        // trace and find where player is aiming
        auto cent = ENTITY(this_->entindex());
        if (CE_BAD(cent) || !cent->hitboxes.GetHitbox(0))
            return weapon;
        Vector eyePos = cent->hitboxes.GetHitbox(0)->center;
        trace::filter_default.SetSelf(this_);
        trace_t trace;
        Ray_t ray;
        QAngle angle = *(QAngle *) &NET_VECTOR(this_, netvar.angEyeAngles);
        if (isLocal)
        {
            angle  = VectorToQAngle(current_user_cmd->viewangles);
            eyePos = g_pLocalPlayer->v_Eye;
        }
        Vector forward;
        AngleVectors2(angle, &forward);
        forward *= 8192.0f;
        forward += eyePos;
        ray.Init(eyePos, forward);
        g_ITrace->TraceRay(ray, MASK_SHOT, &trace::filter_default, &trace);
        data.m_vEnd = trace.endpos;
    }
    data.m_iEffectId = 0xDEADCA7; // handled in other detour
    DispatchEffect_fn("ParticleEffect", data);
    return weapon;
}

typedef void (*FX_Tracer_t)(Vector &, Vector &, int, bool);

FX_Tracer_t FX_Tracer_fn;

char SentryTracerParity[MAX_ENTITIES + 1]{ 0 };
void FX_Tracer_detour(Vector &start, CEffectData &data, int velocity, bool makeWhiz)
{
    // start and end are reversed, justvalvethings.club
    if (!sentry_tracers || !enable)
        return FX_Tracer_fn(start, data.m_vStart, velocity, makeWhiz);
    auto sentry = g_IEntityList->GetClientEntityFromHandle(data.m_hEntity);
    if (!sentry || sentry->entindex() == -1 || sentry->GetClientClass()->m_ClassID != CL_CLASS(CObjectSentrygun))
        return;
    int muzzle   = 4; // level 1
    int muzzle_l = 1; // level 2 & 3
    int muzzle_r = 2; // level 2 & 3
    CEffectData dataTracer;
    if (NET_INT(sentry, netvar.iUpgradeLevel) > 1)
    {
        auto &parity = SentryTracerParity[sentry->entindex()];
        parity       = !parity;
        vfunc<GetAttachment_t>(sentry, 113, 0)(sentry, parity ? muzzle_l : muzzle_r, dataTracer.m_vStart);
    }
    else
        vfunc<GetAttachment_t>(sentry, 113, 0)(sentry, muzzle, dataTracer.m_vStart);

    dataTracer.m_hEntity = sentry->GetRefEHandle();
    // we don't have accurate sentry angles, use bullet tracer end point which is start for some reason,
    // justvalvethings.club
    dataTracer.m_vEnd      = data.m_vStart;
    dataTracer.m_iEffectId = 0xDEADCA7; // handled in other detour
    DispatchEffect_fn("ParticleEffect", dataTracer);
}

#define foffset(p, i) ((unsigned char *) &p)[i]

static InitRoutine init(
    []()
    {
        // Init up here, do NOT patch these, only patch them after setting
        static std::optional<BytePatch> patch;
        static std::optional<BytePatch> patch2;
        static std::optional<BytePatch> patch3;
        enable.installChangeCallback(
            [](settings::VariableBase<bool> &, bool after)
            {
                if (!patch)
                {
                    static auto addr1 = gSignatures.GetClientSignature("8B 43 54 89 04 24 E8 ? ? ? ? F6 43 30 01 89 C7");                          // GetParticleSystemNameFromIndex detour
                    static auto addr2 = gSignatures.GetClientSignature("E8 ? ? ? ? 85 C0 89 C3 0F 84 ? ? ? ? 8B 00 89 1C 24 FF 90 ? ? ? ? 80 BB"); // GetActiveTFWeapon detour
                    static auto addr3 = gSignatures.GetClientSignature("E8 ? ? ? ? 89 F8 84 C0 0F 85 ? ? ? ? 85 F6");
                    static auto addr4 = gSignatures.GetClientSignature("E8 ? ? ? ? 8D 85 ? ? ? ? 89 7C 24 0C 89 44 24 10");
                    static auto addr5 = gSignatures.GetClientSignature("E8 ? ? ? ? 8D 65 F4 5B 5E 5F 5D C3 8D 76 00 8B 43 0C"); // FX_Tracer detour

                    BytePatch::mprotectAddr(addr1 + 1, 4, PROT_READ | PROT_WRITE | PROT_EXEC);
                    BytePatch::mprotectAddr(addr2 + 1, 4, PROT_READ | PROT_WRITE | PROT_EXEC);
                    BytePatch::mprotectAddr(addr3 + 1, 4, PROT_READ | PROT_WRITE | PROT_EXEC);
                    BytePatch::mprotectAddr(addr4 + 1, 4, PROT_READ | PROT_WRITE | PROT_EXEC);
                    BytePatch::mprotectAddr(addr5 + 1, 4, PROT_READ | PROT_WRITE | PROT_EXEC);

                    GetParticleSystemNameFromIndex_fn = GetParticleSystemNameFromIndex_t(e8call(addr1 + 7));
                    GetActiveTFWeapon_fn              = GetActiveTFWeapon_t(e8call_direct(addr2));
                    DispatchEffect_fn                 = DispatchEffect_t(e8call_direct(addr3));
                    CalcZoomedMuzzleLocation_fn       = CalcZoomedMuzzleLocation_t(e8call_direct(addr4));
                    FX_Tracer_fn                      = FX_Tracer_t(e8call_direct(addr5));

                    static auto relAddr1 = ((uintptr_t) GetParticleSystemNameFromIndex__detour - ((uintptr_t) addr1 + 3)) - 5;
                    static auto relAddr2 = ((uintptr_t) GetActiveTFWeapon_detour - ((uintptr_t) addr2)) - 5;
                    static auto relAddr3 = ((uintptr_t) FX_Tracer_detour - ((uintptr_t) addr5)) - 5;

                    patch  = BytePatch((void *) addr1, { 0x89, 0x1C, 0x24, 0xE8, foffset(relAddr1, 0), foffset(relAddr1, 1), foffset(relAddr1, 2), foffset(relAddr1, 3), 0x90, 0x90, 0x90 });
                    patch2 = BytePatch((void *) addr2, { 0xE8, foffset(relAddr2, 0), foffset(relAddr2, 1), foffset(relAddr2, 2), foffset(relAddr2, 3) });
                    patch3 = BytePatch((void *) addr5, { 0xE8, foffset(relAddr3, 0), foffset(relAddr3, 1), foffset(relAddr3, 2), foffset(relAddr3, 3) });
                }

                if (after)
                {
                    (*patch).Patch();
                    (*patch2).Patch();
                    (*patch3).Patch();
                }
                else
                {
                    (*patch).Shutdown();
                    (*patch2).Shutdown();
                    (*patch3).Shutdown();
                }
            });
        /* clang-format on */
        EC::Register(
            EC::Shutdown,
            []()
            {
                if (patch)
                {
                    (*patch).Shutdown();
                    (*patch2).Shutdown();
                    (*patch3).Shutdown();
                }
            },
            "shutdown_bullettrace");
    });
} // namespace hacks::tf2::bullettracers
