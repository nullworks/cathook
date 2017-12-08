
/*
 * logging.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include <stdarg.h> // Infinit arguments, va_lists
#include <string.h> // String stuff
#include <fstream> 	// IO
#include <iostream> // IO
#include <time.h> 	// Time

#include "logging.hpp"

// Cathooks main logging util
CatLogger g_CatLogging("/tmp/cathook.log", true);

CatLogger::CatLogger(const char* file_path, bool _ptime) : log_handle(fopen(file_path, "w")), ptime(_ptime) {}
CatLogger::~CatLogger() {	fclose(log_handle); }

void CatLogger::log(const char* fmt, ...) {

	// Print our time if needed
	if (ptime) {
		// Get our time
		time_t current_time = time(0);
		struct tm* time_info = localtime(&current_time);
		// print it to a string
		char timeString[10];
		strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);

		// Print the time into the log
		fprintf(log_handle, "%% [%s] ", timeString);
	}

	// Get the string we want to log
	char buffer[1024];
	va_list list;
	va_start(list, fmt);
	vsprintf(buffer, fmt, list);
	va_end(list);

	// Write our log to the file
	fprintf(log_handle, "%s\n", buffer);
	fflush(log_handle);

	// Push result var to a console here, if i ever make a console api
}
