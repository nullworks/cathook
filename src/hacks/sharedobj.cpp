
/*
 * sharedobj.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#if defined(__linux__)
#include <dlfcn.h> // dlopen, libary stuffs
#endif
#include <cstring> // char string utils
#include <fstream> // std::ifstream
#include <regex> 	// Regular expressions
#include <chrono> // For time so we can sleep
#include <thread> // Sleep in thread ^

#include "../util/logging.hpp"			// Logging

#include "sharedobj.hpp"

// Input a shared objects name and it attemts to save the full path to the string, returns false if fails.
static std::string LocateSharedObject(const char* name) {
#if defined(__linux__) // This is linux specific

	// Open /proc/maps to get a list of libraries being used currently
	std::ifstream proc_maps("/proc/self/maps");
	if (!proc_maps.is_open())
		return std::string();

	// Recurse through the lines of the file
	while (!proc_maps.eof()) {
		// Get our line
		char buffer[1024];
		proc_maps.getline(buffer, sizeof(buffer));

		// Test if it contains our library
		if (!strstr(buffer, name))
			continue;

		// Extract our objects path
		std::cmatch reg_result;
		if (!std::regex_search(buffer, reg_result, std::regex("\\/([\\s\\w-\\.]+\\/)+" + std::string(name)))) // regex is really slow and adds alot to file size, but is really convienient for this purpose
			continue;
		// Return the path
		return reg_result[0];
	}

	/* This works but the regex is better at making sure its the correct so, EX: with Team Fortress, if you try to find client.so, it would find steamclient.so instead
	// Book keeper
	auto tmp = std::make_pair(name, std::string());

	// This loops through dlls loaded
	if (dl_iterate_phdr([](struct dl_phdr_info* info, size_t, void* data) {
		// Cast our tmp var back
		auto tmp_var = (std::pair<const char*, std::string>*)data;
		// Test if it contains our library
		if (!!strcasestr(info->dlpi_name, tmp_var->first)) {
			// Set tmp var
			tmp_var->second = info->dlpi_name;
			return 1;
		}
		return 0;
	}, (void*)&tmp))
		return tmp.second;*/

#elif defined(_WIN32)
	// TODO, needs better way to do this so that modules dont already need to be loaded
	auto module_handle = GetModuleHandle(name);
	if (!module_handle) return std::string();
	char buffer[1024];
	auto result = GetModuleFileName(module_handle, buffer, sizeof(buffer));
	if (result && result != sizeof(buffer)) return buffer;
#endif
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
#if defined(__linux__)
	while (!(lmap = (CatLinkMap*)dlopen(path.c_str(), RTLD_NOLOAD | RTLD_NOW | RTLD_LOCAL))) {
		auto error = dlerror();
		if (error) g_CatLogging.log("DLERROR: %s", error);
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
#elif defined(_WIN32)

#endif

	g_CatLogging.log("Linkmap: %s -> 0x%x", _file_name, lmap);
}
