
/*
 * entry.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include <pthread.h> // So we can make our unix thread, works in mingw for windows too!

#include "util/logging.hpp" // To log
#include "hack.h"	// Contains main init

#if defined(__linux__)

void __attribute__((constructor)) entry() {
	// A thread so we can attach and let the cheat handle the rest
	pthread_t init_thread;
	pthread_create(&init_thread, 0, [](void*) -> void* {
		hack::Initialize();
		return 0;
	}, 0);
}

#elif defined(_WIN32)

#include <windows.h>

// Uhhh, meme?
BOOL WINAPI DllMain(HMODULE hInstance, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		// Create a thread
		pthread_t init_thread;
		pthread_create(&init_thread, 0, [](void*) -> void* {
			hack::Initialize();
			return 0;
		}, 0);
	}

	return true;
}

#endif
