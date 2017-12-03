
/*
 *
 *	String helpers header
 *
 */

#pragma once

#include <stdarg.h> // Infinite arguments, va_lists
#include <stdio.h>	// vsprintf()

// Give it a buffer, the string to pull from, and the bounds to use. Keep in mind, this expects correct bounds and such.
inline void substr(char* buffer, const char* input, const int& start, const int& end) {
	int needed_length = end - start;
	for (int i = 0; i < needed_length; i++) {
		buffer[i] = input[start + i];
	}
	buffer[needed_length] = '\0';
}
