/*
 * C_TFWeaponBaseMelee.hpp
 *
 *  Created on: Nov 23, 2017
 *      Author: nullifiedcat
 */

#pragma once

class C_TFWeaponBaseMelee: public C_TFWeaponBase
{
public:
    inline static bool DoSwingTrace(IClientEntity *self, trace_t *trace)
    {
        typedef bool(*fn_t)(IClientEntity *, trace_t *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(522, offsets::undefined, 522), 0)(self, trace);
    }
};


