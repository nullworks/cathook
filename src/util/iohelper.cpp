

/*
 *
 *	Do io stuff here
 *
 */
#include <fstream>
#include <ios>
#include <sstream>

#include "iohelper.hpp"

// Gets name of process using unix goodies
std::string GetProcessName() {
	std::ifstream read_stream("/proc/self/status", std::ios::binary);
	if (!read_stream.is_open()) return "unknown";
	read_stream.seekg(0);
	
	char str[30];
	read_stream.getline(str, 30, '\n'); // Stop at newline, also char is gay
	return std::string(str).substr(6,24); // Get rid of "Name: " from the string
}