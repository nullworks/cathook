
/*
 * sharedobj.h
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include <link.h> // link maps
#include <string>	// std::string

// Create a shared object with the name of the shared object, then it attempts to find its path and allows access to the linkmap
class SharedObject {
public:
	SharedObject(const char* _file_name);
	const char* file_name;	// The name of the library we want
 	std::string path = "unassigned"; // The full path of the shared library we are looking for
	link_map* GetLmap(); // Used because we need to init before lmap becomes usable 
private:
	link_map* lmap = nullptr; // Link map returned from dllopen
};
