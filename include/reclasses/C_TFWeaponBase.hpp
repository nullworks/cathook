/*
 * C_TFWeaponBase.hpp
 *
 *  Created on: Nov 23, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <common.hpp>

namespace re
{

class C_TFWeaponBase : public re::C_BaseCombatWeapon
{
public:
    inline static void GetProjectileFireSetup(IClientEntity *weapon, IClientEntity *pPlayer, Vector vecOffset, Vector *vecSrc, Vector *angForward, bool bHitTeammates, float flEndDist)
    {
        typedef void (*GetProjectileFireSetup_t)(IClientEntity * weapon, IClientEntity * pPlayer, Vector vecOffset, Vector * vecSrc, Vector * angForward, bool bHitTeammates, float flEndDist);
        static auto signature                                     = gSignatures.GetClientSignature("55 89 E5 57 56 53 81 EC 9C 01 00 00 8B 75");
        static GetProjectileFireSetup_t GetProjectileFireSetup_fn = (GetProjectileFireSetup_t) signature;
        GetProjectileFireSetup_fn(weapon, pPlayer, vecOffset, vecSrc, angForward, bHitTeammates, flEndDist);
    }
    // Need a seperate one for the Huntsman
    inline static void GetProjectileFireSetupHuntsman(IClientEntity *weapon, IClientEntity *pPlayer, Vector vecOffset, Vector *vecSrc, Vector *angForward, bool bHitTeammates, float flEndDist)
    {
        typedef void (*GetProjectileFireSetupHuntsman_t)(IClientEntity * weapon, IClientEntity * pPlayer, Vector vecOffset, Vector * vecSrc, Vector * angForward, bool bHitTeammates, float flEndDist);
        static auto signature                                                     = gSignatures.GetClientSignature("55 89 E5 56 53 83 EC 30 0F B6 45");
        static GetProjectileFireSetupHuntsman_t GetProjectileFireSetupHuntsman_fn = (GetProjectileFireSetupHuntsman_t) signature;
        GetProjectileFireSetupHuntsman_fn(weapon, pPlayer, vecOffset, vecSrc, angForward, bHitTeammates, flEndDist);
    }
    inline static Vector GetSpreadAngles(IClientEntity *self)
    {
        typedef Vector (*GetSpreadAngles_t)(IClientEntity *);
        static auto signature                       = gSignatures.GetClientSignature("55 89 E5 57 56 53 83 EC 7C 8B 7D ? 8B 5D ? 89 3C 24");
        static GetSpreadAngles_t GetSpreadAngles_fn = (GetSpreadAngles_t) signature;
        return GetSpreadAngles_fn(self);
    }
    inline static int GetWeaponID(IClientEntity *self)
    {
        typedef int (*fn_t)(IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(449, offsets::undefined, 449), 0)(self);
    }
    inline static bool IsViewModelFlipped(IClientEntity *self)
    {
        typedef bool (*fn_t)(IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(498, offsets::undefined, 498), 0)(self);
    }
    inline static IClientEntity *GetOwnerViaInterface(IClientEntity *self)
    {
        typedef IClientEntity *(*fn_t)(IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(456, offsets::undefined, 456), 0)(self);
    }
    inline static bool UsesPrimaryAmmo(IClientEntity *self)
    {
        typedef bool (*fn_t)(IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(452, offsets::undefined, 452), 0)(self);
    }
    inline static bool HasPrimaryAmmo(IClientEntity *self)
    {
        typedef bool (*fn_t)(IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(320, offsets::undefined, 320), 0)(self);
    }
    inline static bool AreRandomCritsEnabled(IClientEntity *self)
    {
        typedef bool (*fn_t)(IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(471, offsets::undefined, 470), 0)(self);
    }
    inline static bool CalcIsAttackCriticalHelper(IClientEntity *self)
    {
        typedef bool (*fn_t)(IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(465, offsets::undefined, 465), 0)(self);
    }
    inline static bool CalcIsAttackCriticalHelperNoCrits(IClientEntity *self)
    {
        typedef bool (*fn_t)(IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(464, offsets::undefined, 464), 0)(self);
    }
    inline static bool CanFireCriticalShot(IClientEntity *self, bool unknown1, IClientEntity *unknown2)
    {
        typedef bool (*fn_t)(IClientEntity *, bool, IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(494, offsets::undefined, 494), 0)(self, unknown1, unknown2);
    }
    inline static float ApplyFireDelay(IClientEntity *self, float delay)
    {
        typedef float (*fn_t)(IClientEntity *, float);
        return vfunc<fn_t>(self, offsets::PlatformOffset(478, offsets::undefined, 478), 0)(self, delay);;
    }
    inline static void AddToCritBucket(IClientEntity *self, float value)
    {
        constexpr float max_bucket_capacity = 1000.0f;
        crit_bucket_(self)                  = fminf(crit_bucket_(self) + value, max_bucket_capacity);
    }
    inline static bool IsAllowedToWithdrawFromCritBucket(IClientEntity *self, float value)
    {
        uint16_t weapon_info_handle = weapon_info_handle_(self);
        void *weapon_info           = nullptr; // GetFileWeaponInfoFromHandle(weapon_info_handle);
        /*
        if (!weapon_info->unk_1736)
        {

        }
        */
    }
    inline static bool CalcIsAttackCriticalHelper_re(IClientEntity *self)
    {
        IClientEntity *owner = GetOwnerViaInterface(self);

        if (owner == nullptr)
            return false;

        if (!C_BaseEntity::IsPlayer(owner))
            return false;

        CTFPlayerShared *shared = &C_BasePlayer::shared_(owner);
        float critmult          = CTFPlayerShared::GetCritMult(shared);
        if (!CanFireCriticalShot(self, 0, nullptr))
            return false;

        if (CTFPlayerShared::IsCritBoosted(shared))
            return true;

        int unk1 = *(int *) (unsigned(self) + 2832u);
        int unk2 = *(int *) (unsigned(self) + 2820u);
        unk2 <<= 6;

        int unk3  = unk1 + unk2 + 1784;
        char unk4 = *(char *) (unk1 + unk2 + 1844);
        if (unk4 && *(float *) (unsigned(self) + 2864u) > g_GlobalVars->curtime)
            return true;

        int unk5         = *(int *) (unk1 + unk2 + 1788);
        int bullet_count = 0;
        if (unk5 > 0)
        {
            // mult_bullets_per_shot
        }
        else
        {
            bullet_count = 1;
        }

        float mult2 = *(float *) (unk3);

        float multiplier = 0.5f;
        int seed         = C_BaseEntity::m_nPredictionRandomSeed() ^ (owner->entindex() | (self->entindex() << 8));
        RandomSeed(seed);

        bool result = true;
        if (multiplier * 10000.0f <= RandomInt(0, 9999))
        {
            result     = false;
            multiplier = 0.0f;
        }

        return false;
    }
    inline static int CalcIsAttackCritical(IClientEntity *self)
    {
        IClientEntity *owner = GetOwnerViaInterface(self);
        if (owner)
        {
            if (C_BaseEntity::IsPlayer(owner))
            {
                // Always run calculations
                // Never write anything into entity, at least from here.

                // if (g_GlobalVars->framecount != *(int *)(self + 2872))
                {
                    // *(int *)(self + 2872) = g_GlobalVars->framecount;
                    // *(char *)(self + 2839) = 0;

                    if (g_pGameRules->roundmode == 5 && g_pGameRules->winning_team == NET_INT(owner, netvar.iTeamNum))
                    {
                        // *(char *)(self + 2838) = 1;
                        return 1;
                    }
                    else
                    {
                        if (AreRandomCritsEnabled(self))
                            return CalcIsAttackCriticalHelper(self);
                        else
                            return CalcIsAttackCriticalHelperNoCrits(self);
                    }
                }
            }
        }

        return 0;
    }
    inline static uint16_t &weapon_info_handle_(IClientEntity *self)
    {
        return *(uint16_t *) (unsigned(self) + 2750u);
    }
    inline static float &crit_bucket_(IClientEntity *self)
    {
        return *(float *) (unsigned(self) + 2616u);
    }
};
} // namespace re
