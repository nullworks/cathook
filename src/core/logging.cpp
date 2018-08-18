/*
 * logging.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include <stdarg.h>
#include <string.h>

#include <pwd.h>
#include <settings/Bool.hpp>

#include "common.hpp"

settings::Bool log_to_console{ "hack.log-console", "false" };

FILE *logging::handle{ nullptr };

void logging::Initialize()
{
    // FIXME other method of naming the file?
    passwd *pwd     = getpwuid(getuid());
    logging::handle = fopen(
        strfmt("/tmp/cathook-%s-%d.log", pwd->pw_name, getpid()).get(), "w");
}

void logging::Info(const char *fmt, ...)
{
    if (logging::handle == nullptr)
        logging::Initialize();
    char *buffer = new char[1024];
    va_list list;
    va_start(list, fmt);
    vsprintf(buffer, fmt, list);
    va_end(list);
    size_t length = strlen(buffer);
    char *result  = new char[length + 24];
    time_t current_time;
    struct tm *time_info = nullptr;
    char timeString[10];
    time(&current_time);
    time_info = localtime(&current_time);
    strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);
    sprintf(result, "%% [%s] %s\n", timeString, buffer);
    fprintf(logging::handle, "%s", result);
    fflush(logging::handle);
#if ENABLE_VISUALS
    if (g_ICvar)
    {
        if (*log_to_console)
            g_ICvar->ConsolePrintf("%s", result);
    }
#endif
    delete[] buffer;
    delete[] result;
}

void logging::Shutdown()
{
    fclose(logging::handle);
    logging::handle = nullptr;
}
