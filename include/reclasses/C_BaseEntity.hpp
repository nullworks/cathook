/*
 * C_BaseEntity.hpp
 *
 *  Created on: Nov 23, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "reclasses.hpp"
#include "copypasted/CSignature.h"
#include "e8call.hpp"

namespace re
{

class C_BaseEntity
{
public:
    inline static bool IsPlayer(IClientEntity *self)
    {
        typedef bool (*fn_t)(IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(184, offsets::undefined, 184), 0)(self);
    }
    inline static bool ShouldCollide(IClientEntity *self, int collisionGroup, int contentsMask)
    {
        typedef bool (*fn_t)(IClientEntity *, int, int);
        return vfunc<fn_t>(self, offsets::PlatformOffset(198, offsets::undefined, 198), 0)(self, collisionGroup, contentsMask);
    }
    inline static int &m_nPredictionRandomSeed()
    {
        static int placeholder = 0;
        return placeholder;
    }
    inline static int SetAbsOrigin(IClientEntity *self, Vector const &origin)
    {
        typedef int (*SetAbsOrigin_t)(IClientEntity *, Vector const &);
        static uintptr_t addr                 = e8call_direct(gSignatures.GetClientSignature("E8 ? ? ? ? EB 7D 8B 42 04"));
        static SetAbsOrigin_t SetAbsOrigin_fn = SetAbsOrigin_t(addr);

        return SetAbsOrigin_fn(self, origin);
    }
};
} // namespace re
