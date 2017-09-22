
#ifndef STRINGHELPER
#define STRINGHELPER

#include <string.h> // String stuff
#include <stdarg.h> // Infinit arguments, va_lists
#include <stdio.h>	// vsprintf()

#include "colors.hpp"


char* strfmt(const char* fmt, ...); 
rgba_t StrToRgba(char* text);
//wchar_t* GetWC(const char* fmt, ...);


#endif