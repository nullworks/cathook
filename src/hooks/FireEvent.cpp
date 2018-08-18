/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include "HookedMethods.hpp"

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(FireEvent, bool, IGameEventManager2 *this_,
                     IGameEvent *event, bool no_broadcast)
{
    return original::FireEvent(this_, event, no_broadcast);
}
} // namespace hooked_methods