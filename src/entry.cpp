
/*
 * entry.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include <thread>

#include "hack.h"	// Contains main init

#if defined(__linux__)

void __attribute__((constructor)) entry() {
	std::thread init_thread(hack::Initialize);
	init_thread.detach();
}

#elif defined(_WIN32)

#include <windows.h>

BOOL WINAPI DllMain(HMODULE hInstance, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		std::thread init_thread(hack::Initialize);
		init_thread.detach();
	}
	return true;
}

#endif
