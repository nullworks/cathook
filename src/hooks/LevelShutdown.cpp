/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <hacks/Aimbot.hpp>
#include <hacks/hacklist.hpp>
#include "HookedMethods.hpp"

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(LevelShutdown, void, void *this_)
{
    hacks::shared::autojoin::resetQueueTimer();
    need_name_change = true;
#if !LAGBOT_MODE
    playerlist::Save();
#endif
    g_Settings.bInvalid = true;
#if !LAGBOT_MODE
    hacks::shared::aimbot::Reset();
    chat_stack::Reset();
    hacks::shared::anticheat::ResetEverything();
#endif
#if ENABLE_IPC
    if (ipc::peer)
    {
        ipc::peer->memory->peer_user_data[ipc::peer->client_id]
            .ts_disconnected = time(nullptr);
    }
#endif
    return original::LevelShutdown(this_);
}
} // namespace hooked_methods
