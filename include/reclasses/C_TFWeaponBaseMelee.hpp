/*
 * C_TFWeaponBaseMelee.hpp
 *
 *  Created on: Nov 23, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "reclasses.hpp"

namespace re
{

class C_TFWeaponBaseMelee : public C_TFWeaponBase
{
public:
    inline static bool DoSwingTrace(IClientEntity *self, trace_t *trace)
    {
        typedef bool (*fn_t)(IClientEntity *, trace_t *);
        return vfunc<fn_t>(
            self, offsets::PlatformOffset(522, offsets::undefined, 522),
            0)(self, trace);
    }
    inline static int GetSwingRange(IClientEntity *self)
    {
        if (self->GetClientClass()->m_ClassID == CL_CLASS(CTFSword) ||
            self->GetClientClass()->m_ClassID == CL_CLASS(CTFKatana))
            return 128;
        else
            return (128.0f * 0.67) - 0.5f;
    }
};
} // namespace re
