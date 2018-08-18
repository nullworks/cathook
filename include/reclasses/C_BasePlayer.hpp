#pragma once

#include "reclasses.hpp"

namespace re
{

class C_BasePlayer : public C_BaseEntity
{
public:
    inline static CTFPlayerShared &shared_(IClientEntity *self)
    {
        return *((CTFPlayerShared *) (unsigned(self) + 6092));
    }
};
} // namespace re
