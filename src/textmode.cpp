/*
 * textmode.cpp
 *
 *  Created on: Jul 28, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

bool *allowSecureServers{ nullptr };

// valve pls no ban

void EXPOSED_Epic_VACBypass_1337_DoNotSteal_xXx_$1_xXx_MLG()
{
    ((ICommandLine * (*) (void) ) dlsym(sharedobj::tier0().lmap, "CommandLine_Tier0"))()->RemoveParm("-textmode");
    ((ICommandLine * (*) (void) ) dlsym(sharedobj::tier0().lmap, "CommandLine_Tier0"))()->RemoveParm("-insecure");
    uintptr_t Host_IsSecureServerAllowed_addr  = gSignatures.GetEngineSignature("55 89 E5 83 EC ? E8 ? ? ? ? 8B 10 C7 44 24 ? ? ? ? ? 89 04 24 FF 52 ? 85 C0 74 ? C6 05");
    uintptr_t Host_IsSecureServerAllowed2_addr = gSignatures.GetEngineSignature("55 89 E5 83 EC ? E8 ? ? ? ? 8B 10 C7 44 24 ? ? ? ? ? 89 04 24 FF 52 ? 85 C0 0F");
    // +0x21 = allowSecureServers
    // logging::Info("1337 VAC bypass: 0x%08x",
    // Host_IsSecureServerAllowed_addr);
    static BytePatch HostSecureServer(Host_IsSecureServerAllowed_addr, { 0x55, 0x89, 0xE5, 0x83, 0xEC, 0x18, 0x31, 0xC0, 0x40, 0xC9, 0xC3 });
    static BytePatch HostSecureServer2(Host_IsSecureServerAllowed2_addr, { 0x31, 0xC0, 0x40, 0xC3 });
    HostSecureServer.Patch();
    HostSecureServer2.Patch();

    uintptr_t allowSecureServers_addr = Host_IsSecureServerAllowed_addr + 0x21;

    BytePatch::mprotectAddr(allowSecureServers_addr, 4, PROT_READ | PROT_WRITE | PROT_EXEC);
    BytePatch::mprotectAddr(*(uintptr_t *) allowSecureServers_addr, 4, PROT_READ | PROT_WRITE | PROT_EXEC);
    BytePatch::mprotectAddr(**(uintptr_t **) allowSecureServers_addr, 4, PROT_READ | PROT_WRITE | PROT_EXEC);

    allowSecureServers = *(bool **) (allowSecureServers_addr);
    logging::Info("Allow Secure Servers: 0x%08x", allowSecureServers);
    *allowSecureServers = true;
    logging::Info("Allow Secure Servers: %d", *allowSecureServers);
}

CatCommand fixvac("fixvac", "Lemme in to secure servers", []() { EXPOSED_Epic_VACBypass_1337_DoNotSteal_xXx_$1_xXx_MLG(); });

static InitRoutine init_textmode([]() {
#if ENABLE_TEXTMODE_STDIN
    logging::Info("[TEXTMODE] Setting up input handling");
    int flags = fcntl(0, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(0, F_SETFL, flags);
    logging::Info("[TEXTMODE] stdin is now non-blocking");
#endif
#if ENABLE_VAC_BYPASS
    EXPOSED_Epic_VACBypass_1337_DoNotSteal_xXx_$1_xXx_MLG();
#endif
});

#if ENABLE_TEXTMODE_STDIN
void UpdateInput()
{
    char buffer[256];
    int bytes = read(0, buffer, 255);
    if (bytes > 0)
    {
        buffer[bytes] = '\0';
        g_IEngine->ExecuteClientCmd(buffer);
    }
}
#endif
