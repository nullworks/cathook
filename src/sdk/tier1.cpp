//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A higher level link library for general use in the game and tools.
//
//===========================================================================//

#include <tier1/tier1.h>
#include "tier0/dbg.h"
#include "vstdlib/iprocessutils.h"
#include "icvar.h"

//-----------------------------------------------------------------------------
// These tier1 libraries must be set by any users of this library.
// They can be set by calling ConnectTier1Libraries or InitDefaultFileSystem.
// It is hoped that setting this, and using this library will be the common
// mechanism for allowing link libraries to access tier1 library interfaces
//-----------------------------------------------------------------------------
ICvar *cvar                    = 0;
ICvar *g_pCVar                 = 0;
IProcessUtils *g_pProcessUtils = 0;
static bool s_bConnected       = false;

// for utlsortvector.h
#ifndef _WIN32
void *g_pUtlSortVectorQSortContext = NULL;
#endif

//-----------------------------------------------------------------------------
// Call this to connect to all tier 1 libraries.
// It's up to the caller to check the globals it cares about to see if ones are
// missing
//-----------------------------------------------------------------------------
void ConnectTier1Libraries(CreateInterfaceFn *pFactoryList, int nFactoryCount)
{
    // Don't connect twice..
    if (s_bConnected)
        return;

    s_bConnected = true;

    for (int i = 0; i < nFactoryCount; ++i)
    {
        if (!g_pCVar)
        {
            cvar = g_pCVar = (ICvar *) pFactoryList[i](CVAR_INTERFACE_VERSION, NULL);
        }
        if (!g_pProcessUtils)
        {
            g_pProcessUtils = (IProcessUtils *) pFactoryList[i](PROCESS_UTILS_INTERFACE_VERSION, NULL);
        }
    }
}

void DisconnectTier1Libraries()
{
    if (!s_bConnected)
        return;

    g_pCVar = cvar  = 0;
    g_pProcessUtils = NULL;
    s_bConnected    = false;
}

// Function definitions for netadr.h
#include "tier1/netadr.h"
#include <arpa/inet.h>

const char *netadr_s::ToString(bool onlyBase) const
{
    static char s[64];

    strncpy(s, "unknown", sizeof(s));

    if (type == NA_LOOPBACK)
    {
        strncpy(s, "loopback", sizeof(s));
    }
    else if (type == NA_BROADCAST)
    {
        strncpy(s, "broadcast", sizeof(s));
    }
    else if (type == NA_IP)
    {
        if (onlyBase)
        {
            snprintf(s, sizeof(s), "%i.%i.%i.%i", ip[0], ip[1], ip[2], ip[3]);
        }
        else
        {
            snprintf(s, sizeof(s), "%i.%i.%i.%i:%i", ip[0], ip[1], ip[2], ip[3], ntohs(port));
        }
    }

    return s;
}

bool netadr_t::IsReservedAdr() const
{
    if (type == NA_LOOPBACK)
        return true;

    if (type == NA_IP)
    {
        if ((ip[0] == 10) ||                                // 10.x.x.x is reserved
            (ip[0] == 127) ||                               // 127.x.x.x
            (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31) || // 172.16.x.x  - 172.31.x.x
            (ip[0] == 192 && ip[1] >= 168))                 // 192.168.x.x
            return true;
    }
    return false;
}
