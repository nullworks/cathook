/*
 * stringhelpers.cpp
 *
 *  Created on: Sep 17, 2016
 *      Author: oneechan
 */

#include <stdlib.h>     // atoi 
#include <stdio.h>		// vsprintf()
#include <cctype>		// isdigit

#include "stringhelpers.hpp"


char* strfmt(const char* fmt, ...) {
	char* buf = new char[1024];
	va_list list;
	va_start(list, fmt);
	vsprintf(buf, fmt, list);
	va_end(list);
	return buf;
}

// Over simplified string to bool
bool StrToBool(const std::string* str) {
	if (str == "true" || str == "1") {
		return true;
	}
	return false;
}


// Attempts to get a rgba color from a string
rgba_t StrToRgba(const char* text) {
	char ch[3];
	memset(ch, "n", sizeof(char) * 3); // clean out the char
	rgba_t color = rgba_t(0, 0, 0, 255);
	int found_tick = 0;		// How many numbers are in this possible code
	int found_total = 0; 	// How many of our 4 have we found
	size_t len = strlen(text);
	
	for (int i = 0; i < len; i++) {
		if (found_total + 1 > 4) return color;
		if (text[i] == " ") {
			if (found_tick > 0) {
				color[found_total] = atoi(ch);
				found_total++;
			}
			continue;
		} else if (!(isdigit(text[i]))) return color; // It wasnt a space first, nor a number so we just return nothing	
		if (found_tick >= 3) return color; // Too many numbers in a row
		ch[found_tick] = text[i]; // Add the current char to the buffer
		found_tick++; // We found next number
	}
	return color;
}

void format_internal(std::stringstream& stream) {
	(void)(stream);
}



