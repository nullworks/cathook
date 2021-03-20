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
        return vfunc<fn_t>(self, offsets::PlatformOffset(523, offsets::undefined, 523), 0)(self, trace);
    }
    inline static int GetSwingRange(IClientEntity *self)
    {
        if (g_pLocalPlayer->holding_sapper)
            return 115;
        IClientEntity *owner = re::C_TFWeaponBase::GetOwnerViaInterface(self);
        bool add_charging    = false;
        if (owner)
        {
            CachedEntity *owner_ce = ENTITY(owner->entindex());
            if (HasCondition<TFCond_Charging>(owner_ce))
            {
                CondBitSet<TFCond_Charging, false>(CE_VAR(owner_ce, netvar.iCond, condition_data_s));
                add_charging = true;
            }
        }
        typedef int (*fn_t)(IClientEntity *);
        int return_value = vfunc<fn_t>(self, offsets::PlatformOffset(521, offsets::undefined, 521), 0)(self);

        if (add_charging)
        {
            CachedEntity *owner_ce = ENTITY(owner->entindex());
            CondBitSet<TFCond_Charging, true>(CE_VAR(owner_ce, netvar.iCond, condition_data_s));
        }

        return_value = ATTRIB_HOOK_FLOAT(return_value, "melee_range_multiplier", RAW_ENT(LOCAL_W), 0x0, true);

        return return_value;
    }
};
} // namespace re
