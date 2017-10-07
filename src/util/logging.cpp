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
#include <sys/types.h>	// Time stuff 
#include <unistd.h> // Gets user info
#include <pwd.h>	// Struct passwd

#include "../shutdown.hpp"

#include "logging.h"

// TODO, add to registered shutdown functions

FILE* log_handle = 0;

void Shutdown() {
	fclose(log_handle);
	log_handle = 0;
}

void Initialize() {
	// FIXME other method of naming the file? Currently uses username for it.
	passwd* pwd = getpwuid(getuid());
	log_handle = fopen(strfmt("/tmp/cathook-%s.log", pwd->pw_name), "w");
	RegisterShutdown(Shutdown);
}

void CatLogging(const char* fmt, ...) {
	if (log_handle == 0) Initialize(); // Check if file is open
	
	char* buffer = new char[1024];
	va_list list;
	va_start(list, fmt);
	vsprintf(buffer, fmt, list);
	va_end(list);
	
	size_t length = strlen(buffer);
	char* result = new char[length + 24];
	
	time_t current_time;
	struct tm * time_info = nullptr;
	char timeString[10];
	time(&current_time);
	time_info = localtime(&current_time);
	strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);
	
	sprintf(result, "%% [%s] %s\n", timeString, buffer);
	fprintf(log_handle, "%s", result);
	fflush(log_handle);
	// Push result var to a console here
	delete [] buffer;
	delete [] result;
}
