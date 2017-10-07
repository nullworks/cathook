
#ifndef STRINGHELPER
#define STRINGHELPER

#include <string.h> // String stuff
#include <stdarg.h> // Infinit arguments, va_lists
#include <sstream>	// For formatting to work properly

#include "colors.hpp"


char* strfmt(const char* fmt, ...); 
//bool StrToBool(std::string str);
//rgba_t StrToRgba(const char* text);

void format_internal(std::stringstream& stream);
template<typename T, typename... Targs>
void format_internal(std::stringstream& stream, T value, Targs... args) {
	stream << value;
	format_internal(stream, args...);
}
template<typename... Args>
std::string format(const Args&... args) {
	std::stringstream stream;
	format_internal(stream, args...);
	return stream.str();
}
#endif