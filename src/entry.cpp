
/*
 * entry.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

// #include <thread>

#include "util/logging.hpp" // To log
#include "hack.h"	// Contains main init

// Attach and detach to use independant of platform
void Attach() {
	g_CatLogging.log("Attaching...");
	hack::Initialize();
	// Freezes on inject so its commented
	/*std::thread init_thread(hack::Initialize)
	init_thread.join();*/
}
void Detach() {
	g_CatLogging.log("Detaching");
}

#if !defined(CMAKE_SYSTEM_NAME) || CMAKE_SYSTEM_NAME == Linux

#include <pthread.h> // So we can make our unix thread

void __attribute__((constructor)) attach() {
	// We need a thread to make things not fail
	pthread_t init_thread;
	pthread_create(&init_thread, 0, [](void*) -> void* { Attach(); return 0; }, 0); // sleep(1) is need to allow the constructors to do their things
}
void __attribute__((destructor)) detach() {
	// Same with the destructor
	pthread_t kill_thread;
	pthread_create(&kill_thread, 0, [](void*) -> void* { Detach(); return 0; }, 0);
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
