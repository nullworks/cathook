
/*
 * sharedobj.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include <dlfcn.h>	// dlopen, libary stuff
#include <cstring>	// char string utils
#include <fstream> // std::ifstream
#include <regex> // Regular expressions
#include <chrono> // For time so we can sleep
#include <thread> // Sleep in thread ^

#include "../util/logging.hpp"			// Logging

#include "sharedobj.hpp"

// Credits to Fission, but idk if Im gonna impliment it...
/*char found[1024];
auto search_directory = [](const char *to_find, const char *dirname, char *out) -> bool {
		auto d = opendir(dirname);
		assert(d);
		defer(closedir(d));

		auto dir = readdir(d);
		while (dir != nullptr) {
				if (dir->d_type == DT_REG && strstr(dir->d_name, to_find) != nullptr && strstr(dir->d_name, ".so") != nullptr) {
						sprintf(out, "%s/%s", dirname, dir->d_name);
						return true;
				}
				dir = readdir(d);
		}
		return false;
};

if (search_directory(name, ".", found) ||
		search_directory(name, "./tf/bin", found) ||
		search_directory(name, "./bin", found)) {
		auto handle = dlopen(found, RTLD_NOLOAD);
		if (handle == nullptr) {
				printf("force loading library %s\n", name);
				handle = dlopen(found, RTLD_NOW);
		}
		return handle;
}*/

// Input a shared objects name and it attemts to save the full path to the string, returns false if fails.
static std::string LocateSharedObject(const char* name) {
	// Open /proc/maps to get a list of libraries being used currently
	std::ifstream proc_maps("/proc/self/maps");
	if (!proc_maps.is_open())
		return std::string();

	// Recurse through the lines of the file
	while (!proc_maps.eof()) {
		// Get our line
		char buffer[512];
		proc_maps.getline(buffer, sizeof(buffer));

		// Test if it contains our library
		if (!strstr(buffer, name))
			continue;

		// Extract our objects path
		std::cmatch reg_result;
		if (!std::regex_search(buffer, reg_result, std::regex("\\/([\\s\\w-]+\\/)+" + std::string(name)))) // regex is really slow and adds alot to file size, but is really convienient for this purpose
			continue;
		// Return the path
		return reg_result[0];
	}
	// We havent found our lib
	return std::string();
}

SharedObject::SharedObject(const char* _file_name) : file_name(_file_name) {

	// Get the full path of our opened object
	auto path = LocateSharedObject(_file_name);
	// Check if we didnt get anything from above, if so retry the above untill we get something
	while (path.empty()) {
		g_CatLogging.log("Didnt find shared object: %s, Retrying!", _file_name);
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
		path = LocateSharedObject(_file_name);
	}

	g_CatLogging.log("Shared object Path: %s -> \"%s\"", _file_name, path.c_str());

	// dlopen that sucker and give us a linkmap to use
	while (!(lmap = (link_map*)dlopen(path.c_str(), RTLD_NOLOAD | RTLD_NOW | RTLD_LOCAL))) {
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
		char* error = dlerror();
		if (error) g_CatLogging.log("DLERROR: %s", error);
	}
	g_CatLogging.log("Linkmap: %s -> 0x%x", _file_name, lmap);
}
