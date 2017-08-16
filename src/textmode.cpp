/*
 * textmode.cpp
 *
 *  Created on: Jul 28, 2017
 *      Author: nullifiedcat
 */

#include "common.h"
#include "init.hpp"
#include "copypasted/CSignature.h"
#include "cvwrapper.h"

#include <fcntl.h>
#include <unistd.h>

bool *allowSecureServers { nullptr };

void EXPOSED_Epic_VACBypass_1337_DoNotSteal_xXx_$1_xXx_MLG() {
	static unsigned char patch[] = { 0x55, 0x89, 0xE5, 0x83, 0xEC, 0x18, 0xB8, 0x01, 0x00, 0x00, 0x00, 0xC9, 0xC3 };
	uintptr_t Host_IsSecureServerAllowed_addr = gSignatures.GetEngineSignature("55 89 E5 83 EC 18 E8 ? ? ? ? 8B 10 C7 44 24 04 ? ? ? ? 89 04 24 FF 52 2C 85 C0 74 11 C6 05 90 43 88 00 00");
	// +0x21 = allowSecureServers
	logging::Info("1337 VAC bypass: 0x%08x", Host_IsSecureServerAllowed_addr);
	Patch((void*)Host_IsSecureServerAllowed_addr, (void*)patch, sizeof(patch));
	uintptr_t allowSecureServers_addr = Host_IsSecureServerAllowed_addr + 0x21;
	allowSecureServers = *(bool**)(allowSecureServers_addr);
	logging::Info("Allow Secure Servers: 0x%08x", allowSecureServers);
	*allowSecureServers = true;
	logging::Info("Done..?");
}

CatCommand fixvac("fixvac", "Lemme in to secure servers", []() {
	*allowSecureServers = true;
});

#ifdef TEXTMODE
InitRoutine init([]() {
#ifdef TEXTMODE_STDIN
	logging::Info("[TEXTMODE] Setting up input handling");
	int flags = fcntl(0, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(0, F_SETFL, flags);
	logging::Info("[TEXTMODE] stdin is now non-blocking");
#endif
	EXPOSED_Epic_VACBypass_1337_DoNotSteal_xXx_$1_xXx_MLG();
});
#endif

void UpdateInput() {
	char buffer[256];
	int bytes = read(0, buffer, 255);
	if (bytes > 0) {
		buffer[bytes] = '\0';
		g_IEngine->ExecuteClientCmd(buffer);
	}
}
