
/*
 * sharedobj.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include <unistd.h>	// sleep()
#include <dlfcn.h>	// dlopen, libary stuff
#include <string.h>	// char string utils

#include "../util/logging.hpp"			// Logging

#include "sharedobj.hpp"

namespace hacks {

// Input a shared objects name and it attemts to save the full path to the string, returns false if fails.
static bool LocateSharedObject(const char* name, std::string& out_full_path) {
	FILE* proc_maps = fopen("/proc/self/maps", "r");
	if (proc_maps == nullptr) return false;
	char buffer[512];
	while (fgets(buffer, 511, proc_maps)) {
		char* path = strchr(buffer, '/');
		char* filename = strrchr(buffer, '/') + 1;
		if (!path || !filename) continue;
		if (!strncmp(name, filename, strlen(name))) {
			out_full_path.assign(path);
			out_full_path.resize(out_full_path.size() - 1);
			return true;
		}
	}
	return false;
}

}

SharedObject::SharedObject(const char* _file_name) : file_name(_file_name) {
	// Get the full path of our opened object
	while (!hacks::LocateSharedObject(_file_name, path)) {
		sleep(1);
	}
	// dlopen that sucker and give us a linkmap to use
	while (!(lmap = (link_map*)dlopen(path.c_str(), RTLD_NOLOAD | RTLD_NOW | RTLD_LOCAL))) {
		sleep(1);
		char* error = dlerror();
		if (error) g_CatLogging.log("DLERROR: %s", error);
	}
}
