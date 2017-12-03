
/*
 *
 *	Cathook global interface util
 *
 */

#pragma once

#include <string>
#include <sstream>
#include <unistd.h>

#include "sharedobj.h"			// We get our interfaces from inside sharedobjects
#include "../util/logging.h"

// MOVE THIS TO SOURCE MODULE

namespace hacks {

// Use to brute force an interface
/*template<typename T>
T* BruteforceInterface(std::string name, sharedobj::SharedObject& object, int start = 0) {
	T* result = nullptr;
	std::stringstream stream;
	for (int i = start; i < 100; i++) {
		stream.str("");
		stream << name;
		int zeros = 0;
		if (i < 10) zeros = 2;
		else if (i < 100) zeros = 1;
		for (int j = 0; j < zeros; j++) stream << '0';
		stream << i;
		result = reinterpret_cast<T*>(object.CreateInterface(stream.str()));
		if (result) return result;
	}
	CatLogging("RIP Software: can't create interface %s!", name.c_str());
	return nullptr;
}*/

}
