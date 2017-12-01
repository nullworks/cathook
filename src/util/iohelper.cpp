

/*
 *
 *	Do io stuff here
 *
 */

#include <fstream>
#include <ios>
#include <sstream>
#include "stringhelpers.hpp"

#include "iohelper.hpp"

// TODO, phase this out and make this need to be done on need of it
// Gets name of process using unix goodies
const char* GetProcessName() {
	std::ifstream read_stream("/proc/self/status", std::ios::binary);
	if (!read_stream.is_open()) return "unknown";
	read_stream.seekg(0);

	static char str[30];
	read_stream.getline(str, 30, '\n'); // Stop at newline, also char is gay
	substr(str, str, 6, 24); // Get rid of "Name: " from the string
	return buf;
}

// Uhhhhh, TODO!!!!
PackedFile::PackedFile(const char* _name, const char* _packed_file) name(_name), packed_file(_packed_file) {

}
