
/*
 * sharedobj.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#if defined(__linux__)
#include <dlfcn.h> // dlopen, libary stuffs
#elif defined(__WIN32)
#include <windows.h>
#endif
#include <cstring> // char string utils
#include <fstream> // std::ifstream
#include <regex> 	// Regular expressions
#include <chrono> // For time so we can sleep
#include <thread> // Sleep in thread ^

#include "../util/logging.hpp"			// Logging

#include "sharedobj.hpp"

void SharedObject::RefreshPath() {
	if (!this->path.empty())
		return;
	#if defined(__linux__)
		std::ifstream proc_maps("/proc/self/maps"); // this contains shared library locations and names
		while (!proc_maps.eof()) {
			char buffer[1024];
			proc_maps.getline(buffer, sizeof(buffer));
			std::cmatch reg_result; // Regex is foolproof for this use
			if (std::regex_search(buffer, reg_result, std::regex(std::string("\\/([\\s\\w-\\.]+\\/)+") + this->file_name))) {
				proc_maps.close();
				this->path = reg_result[0];
				return;
			}
		}
		proc_maps.close();
	#elif defined(_WIN32)
	  this->RefreshLmap();
		char buffer[1024];
		auto result = GetModuleFileName(this->lmap, buffer, sizeof(buffer));
		if (result && result == strlen(buffer))
			this->path = buffer;
	#else
		#warning "Shared object cannot get library name"
	#endif
}

void SharedObject::RefreshLmap(){
	if (this->lmap)
		return;
	#if defined(__linux__)
		while (!(this->lmap = (CatLinkMap_t)dlopen(path.c_str(), RTLD_NOLOAD | RTLD_NOW | RTLD_LOCAL))) {
			auto error = dlerror();
			if (error) g_CatLogging.log("DLERROR: %s", error);
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
	#elif defined(_WIN32)
		while (!(this->lmap = GetModuleHandle(this->file_name))) {
			auto err = GetLastError();
			if (err) g_CatLogging.log("DLERROR: %i", err);
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
	#endif
}

SharedObject::SharedObject(const char* _file_name) : file_name(_file_name) {

	// Get Path
	this->RefreshPath();
	g_CatLogging.log("Shared object Path: %s -> \"%s\"", this->file_name, this->path.c_str());

	// Get handle
	this->RefreshLmap();
	g_CatLogging.log("Shared Object Handle: %s -> 0x%x", this->file_name, this->lmap);
}
