/*
 * C_BaseEntity.hpp
 *
 *  Created on: Nov 23, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "reclasses.hpp"

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
};
}
