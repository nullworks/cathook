/*
 * stringhelpers.cpp
 *
 *  Created on: Sep 17, 2016
 *      Author: oneechan
 */

#include <string.h> // String stuff
#include <stdarg.h> // Infinit arguments, va_lists
#include <stdio.h>	// vsprintf()

#include "stringhelpers.hpp"


char* strfmt(const char* fmt, ...) {
	char* buf = new char[1024];
	va_list list;
	va_start(list, fmt);
	vsprintf(buf, fmt, list);
	va_end(list);
	return buf;
}