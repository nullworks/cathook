/*
 * stringhelpers.cpp
 *
 *  Created on: Sep 17, 2016
 *      Author: oneechan
 */

#include "stringhelpers.hpp"


char* strfmt(const char* fmt, ...) {
	char* buf = new char[1024];
	va_list list;
	va_start(list, fmt);
	vsprintf(buf, fmt, list);
	va_end(list);
	return buf;
}

// Pasted from internet. Dont know how good it is.
/*wchar_t* GetWC(const char* fmt, ...) {
	wchar_t buf = new wchar_t[1024];
	va_list list;
	va_start(list, fmt);
	vsprintf(buf, fmt, list);
	va_end(list);
	return buf;
}*/