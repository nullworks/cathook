/*
 * C_BaseEntity.hpp
 *
 *  Created on: Nov 23, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "reclasses.hpp"
#include "copypasted/CSignature.h"

namespace re
{

class C_BaseEntity
{
public:
    inline static bool IsPlayer(IClientEntity *self)
    {
        typedef bool (*fn_t)(IClientEntity *);
        return vfunc<fn_t>(
            self, offsets::PlatformOffset(184, offsets::undefined, 184),
            0)(self);
    }
    inline static int &m_nPredictionRandomSeed()
    {
        static int placeholder = 0;
        return placeholder;
    }
    inline static int SetAbsOrigin(IClientEntity *self, Vector const &origin)
    {
        typedef int (*SetAbsOrigin_t)(IClientEntity *, Vector const &);
        uintptr_t addr = gSignatures.GetClientSignature(
            "55 89 E5 57 56 53 83 EC ? 8B 5D ? 8B 75 ? 89 1C 24 E8 ? ? ? ? F3 "
            "0F 10 06");
        SetAbsOrigin_t SetAbsOrigin_fn = SetAbsOrigin_t(addr);

        return SetAbsOrigin_fn(self, origin);
    }
};
} // namespace re
