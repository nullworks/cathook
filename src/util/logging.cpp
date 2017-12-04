
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
CatLogger g_CatLogging("/tmp/cathook.log");

CatLogger::CatLogger(const char* file_path) : log_handle(fopen(file_path, "w")) {}
CatLogger::~CatLogger() {	fclose(log_handle); }

void CatLogger::log(const char* fmt, ...) {

	// Get the string we want to log
	char buffer[1024];
	va_list list;
	va_start(list, fmt);
	vsprintf(buffer, fmt, list);
	va_end(list);

	// Get our time
	time_t current_time = time(0);
	struct tm* time_info = localtime(&current_time);
	// print it to a string
	char timeString[10];
	strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);

	// Format the time into the log string
	char result[strlen(buffer) + strlen(timeString)];
	sprintf(result, "%% [%s] %s\n", timeString, buffer);

	// Write our log to the file
	fprintf(log_handle, "%s", result);
	fflush(log_handle);

	// Push result var to a console here, if i ever make a console api
}
