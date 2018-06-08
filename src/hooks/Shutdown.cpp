/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <hacks/hacklist.hpp>
#include "HookedMethods.hpp"

static CatVar die_if_vac(CV_SWITCH, XORSTR("die_if_vac"), XORSTR("0"), XORSTR("Die if VAC banned"));
static CatVar autoabandon(CV_SWITCH, XORSTR("cbu_abandon"), XORSTR("0"), XORSTR("Auto abandon"));
namespace hooked_methods
{
Timer t{};
DEFINE_HOOKED_METHOD(Shutdown, void, INetChannel *this_, const char *reason)
{
    g_Settings.bInvalid = true;
    // This is a INetChannel hook - it SHOULDN'T be static because netchannel
    // changes.
    logging::Info(XORSTR("Disconnect: %s"), reason);
    if (strstr(reason, XORSTR("banned")))
    {
        if (die_if_vac)
        {
            logging::Info(XORSTR("VAC banned"));
            *(int *) 0 = 0;
            exit(1);
        }
    }
#if ENABLE_IPC
    ipc::UpdateServerAddress(true);
#endif
    if (cathook && (disconnect_reason.convar_parent->m_StringLength > 3) &&
        strstr(reason, XORSTR("user")))
    {
        original::Shutdown(this_, disconnect_reason_newlined);
    }
    else
    {
        original::Shutdown(this_, reason);
    }

    if (autoabandon)
    {
        t.update();
        while (1)
            if (t.test_and_set(5000))
            {
                tfmm::dcandabandon();
                break;
            }
    }
    if (hacks::shared::autojoin::auto_queue)
        tfmm::queue_start();
}
}
