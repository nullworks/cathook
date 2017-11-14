
/*
 *
 *	String helpers header
 *
 */

#pragma once

#include <stdarg.h> // Infinite arguments, va_lists
#include <stdio.h>	// vsprintf()

// Input a char array to use as buffer, then the format with variables afterwards to fill the buffer with
inline void strfmt(char* buffer, const char* fmt, ...) {
	va_list list;
	va_start(list, fmt);
	vsprintf(buffer, fmt, list);
	va_end(list);
}

// Give it a buffer, the string to pull from, and the bounds to use. Keep in mind, this expects correct bounds and such.
inline void substr(char* buffer, const char* input, const int& start, const int& end) {
	int needed_length = end - start;
	for (int i = 0; i < needed_length; i++) {
		buffer[i] = input[start + i];
	}
	buffer[needed_length] = '\0';
}