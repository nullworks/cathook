
/*
 *
 *	String helpers header
 *
 */

#pragma once

#include <cstring> // strncpy()
#include <string>
#include <vector>

// Give it a buffer, the string to pull from, and the bounds to use. Keep in mind, this expects correct bounds and such.
inline void substr(char* buffer, const char* input, int start, int end) {
	strncpy(buffer, &input[start], end);
}

// Seperates multiple strings from one string, seperated by spaces
std::vector<std::string> sepstr(const std::string&);

// Wrap a string based on font size
std::string wordwrap(const std::string& in, int max_x);
