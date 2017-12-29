
/*
 * entry.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include <chrono> // For time so we can sleep
#include <thread>

#include "util/logging.hpp" // To log
#include "hack.h"	// Contains main init

#if !defined(CMAKE_SYSTEM_NAME) || CMAKE_SYSTEM_NAME == Linux

#include <pthread.h> // So we can make our unix thread

void __attribute__((constructor)) entry() {
	// A thread so we can attach and let the cheat handle the rest
	pthread_t init_thread;
	pthread_create(&init_thread, 0, [](void*) -> void* {
		// We wait to allow the constructors to finish before we initialize
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		hack::Initialize();
		return 0;
	}, 0);
}

#elif CMAKE_SYSTEM_NAME == Windows

// Uhhh, meme?
BOOL WINAPI DllMain(HMODULE hInstance, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH)
		hack::Initialize(); // TODO, thread this with windows

	return true;
}

#endif
