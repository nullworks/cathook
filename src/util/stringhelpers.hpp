
/*
 *
 *	String helpers header
 *
 */

#pragma once

#include <string.h>

// Give it a buffer, the string to pull from, and the bounds to use. Keep in mind, this expects correct bounds and such.
inline void substr(char* buffer, const char* input, const int& start, const int& end) {
	strncpy(buffer, &input[start], end - start);
}
