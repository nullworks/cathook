#pragma once

#include "reclasses.hpp"
#include "e8call.hpp"

#define CTF_PLR_SHRD 6356
namespace re
{

class CTFPlayerShared
{
public:
    // Convert IClientEntity to CTFPlayerShared
    inline static CTFPlayerShared *GetPlayerShared(IClientEntity *ent)
    {
        return (CTFPlayerShared *) (((uintptr_t) ent) + CTF_PLR_SHRD);
    }
    inline static bool IsDominatingPlayer(CTFPlayerShared *self, int ent_idx)
    {
        static auto signature = e8call_direct(gSignatures.GetClientSignature("E8 ? ? ? ? 84 C0 74 8F"));
        typedef bool (*IsDominatingPlayer_t)(CTFPlayerShared *, int);
        static IsDominatingPlayer_t IsDominatingPlayer_fn = (IsDominatingPlayer_t) signature;
        return IsDominatingPlayer_fn(self, ent_idx);
    }
    inline static float GetCritMult(CTFPlayerShared *self)
    {
        float flRemapCritMul = RemapValClamped(*(int *) ((uintptr_t) self + 672), 0, 255, 1.0, 4.0);
        return flRemapCritMul;
    }
    inline static bool IsCritBoosted(CTFPlayerShared *self)
    {
        static auto signature = gSignatures.GetClientSignature("55 89 E5 57 56 53 83 EC 6C 8B 7D ? C7 44 24 ? 0B 00 00 00");
        typedef bool (*IsCritBoosted_t)(CTFPlayerShared *);
        static IsCritBoosted_t IsCritBoosted_fn = (IsCritBoosted_t) signature;
        return IsCritBoosted_fn(self);
    }
    // Get max charge turning (yaw) to prevent glitchy movement
    inline static float CalculateChargeCap(CTFPlayerShared *self)
    {
        static auto signature = gSignatures.GetClientSignature("55 89 E5 57 56 53 83 EC 6C C7 45 ? 00 00 00 00 8B 45 ? C7 45 ? 00 00 00 00 8B 98");
        typedef float (*CalculateChargeCap_t)(re::CTFPlayerShared *);
        static CalculateChargeCap_t CalculateChargeCap_fn = CalculateChargeCap_t(signature);
        return CalculateChargeCap_fn(self);
    }
    // Get Charge meter (for demoknight things)
    inline static float GetChargeMeter(CTFPlayerShared *self)
    {
        return *(float *) (((uintptr_t) self) + 0x204);
    }
};
} // namespace re
