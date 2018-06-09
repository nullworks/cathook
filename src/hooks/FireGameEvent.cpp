/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include "HookedMethods.hpp"

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(FireGameEvent, void, void *this_, IGameEvent *event)
{
    const char *name = event->GetName();
    if (name)
    {
        if (event_log)
        {
            if (!strcmp(name, XORSTR("player_connect_client")) ||
                !strcmp(name, XORSTR("player_disconnect")) ||
                !strcmp(name, XORSTR("player_team")))
            {
                return;
            }
        }
        //		hacks::tf2::killstreak::fire_event(event);
    }
    original::FireGameEvent(this_, event);
}
}