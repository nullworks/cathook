
/*
 * entry.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include "util/logging.hpp" // To log
#include "hack.h"					// Contains init and shutdown

// Attach and detach to use independant of platform
void Attach() {
	static bool did_do = false;
	if (did_do) return;
	g_CatLogging.log("Attaching...");
	hack::Initialize();
	g_CatLogging.log("Init done...");
}
void Detach() {
	static bool did_do = false;
	if (did_do) return;
	g_CatLogging.log("Detaching");
}

#if !defined(CMAKE_SYSTEM_NAME) || CMAKE_SYSTEM_NAME == Linux

void __attribute__((constructor)) attach() {
	Attach();
}
void __attribute__((destructor)) detach() {
	Detach();
}

#elif CMAKE_SYSTEM_NAME == Windows

// Uhhh, meme?
BOOL WINAPI DllMain(HMODULE hInstance, DWORD dwReason, LPVOID lpReserved) {
	switch(dwReason) {
	case DLL_PROCESS_ATTACH:
		Attach(); break;
	case DLL_PROCESS_DETACH:
		Detach(); break;
	}
	return true;
}

#endif
