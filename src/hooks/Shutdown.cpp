/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <hacks/hacklist.hpp>
#include <settings/Bool.hpp>
#include "HookedMethods.hpp"

static settings::Bool die_if_vac{ "misc.die-if-vac", "false" };
static settings::Bool autoabandon{ "misc.auto-abandon", "false" };
static settings::String custom_disconnect_reason{ "misc.disconnect-reason",
                                                  "" };

namespace hooked_methods
{
Timer t{};
DEFINE_HOOKED_METHOD(Shutdown, void, INetChannel *this_, const char *reason)
{
    g_Settings.bInvalid = true;
    // This is a INetChannel hook - it SHOULDN'T be static because netchannel
    // changes.
    logging::Info("Disconnect: %s", reason);
    if (strstr(reason, "banned"))
    {
        if (die_if_vac)
        {
            logging::Info("VAC banned");
            *(int *) 0 = 0;
            exit(1);
        }
    }
#if ENABLE_IPC
    ipc::UpdateServerAddress(true);
#endif
    if (isHackActive() && (custom_disconnect_reason.toString().size() > 3) &&
        strstr(reason, "user"))
    {
        original::Shutdown(this_, custom_disconnect_reason.toString().c_str());
    }
    else
    {
        original::Shutdown(this_, reason);
    }

    if (autoabandon)
    {
        t.update();
        tfmm::disconnectAndAbandon();
    }
    hacks::shared::autojoin::onShutdown();
}
} // namespace hooked_methods
