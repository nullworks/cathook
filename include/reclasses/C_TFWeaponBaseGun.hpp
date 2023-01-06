/*
 * C_TFWeaponBaseGun.hpp
 *
 *  Created on: Nov 23, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "reclasses.hpp"

namespace re
{

class C_TFWeaponBaseGun : public C_TFWeaponBase
{
public:
    inline static float GetProjectileSpeed(IClientEntity *self)
    {
        typedef float (*fn_t)(IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(539, offsets::undefined, 539), 0)(self);
    }
    inline static float GetWeaponSpread(IClientEntity *self)
    {
        typedef float (*fn_t)(IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(537, offsets::undefined, 537), 0)(self);
    }
    inline static float GetProjectileGravity(IClientEntity *self)
    {
        typedef float (*fn_t)(IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(540, offsets::undefined, 540), 0)(self);
    }
    inline static int LaunchGrenade(IClientEntity *self)
    {
        typedef int (*fn_t)(IClientEntity *);
        return vfunc<fn_t>(self, offsets::PlatformOffset(553, offsets::undefined, 553), 0)(self);
    }
};
} // namespace re
