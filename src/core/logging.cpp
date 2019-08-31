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
#include "hack.hpp"
#include "MiscTemporary.hpp"

static settings::Boolean log_to_console{ "hack.log-console", "false" };

static bool shut_down = false;
std::ofstream logging::handle{ nullptr };

#if ENABLE_LOGGING
void logging::Initialize()
{
    // FIXME other method of naming the file?
    static passwd *pwd = getpwuid(getuid());
    logging::handle.open(strfmt("/tmp/cathook-%s-%d.log", pwd->pw_name, getpid()).get(), std::ios::out | std::ios::app);
}
#endif

void logging::Info(const char *fmt, ...)
{
#if ENABLE_LOGGING
    if (shut_down)
        return;
    if (!handle.is_open())
        logging::Initialize();
    // Argument list
    va_list list;
    va_start(list, fmt);
    // Allocate buffer
    auto result = std::make_unique<char[]>(512);
    // Fill buffer
    if (vsnprintf(result.get(), 512, fmt, list) < 0)
        return;
    va_end(list);

    std::string print_file(result.get());

    time_t current_time;
    struct tm *time_info = nullptr;
    char timeString[10];
    time(&current_time);
    time_info = localtime(&current_time);
    strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);

    std::string to_log = result.get();
    to_log             = strfmt("[%s] ", timeString).get() + to_log + "\n";
    logging::handle << to_log;
    logging::handle.flush();
#if ENABLE_VISUALS
    if (!hack::shutdown)
    {
        if (*log_to_console)
            g_ICvar->ConsoleColorPrintf(MENU_COLOR, "CAT: %s\n", result.get());
    }
#endif
#endif
}

void logging::Shutdown()
{
#if ENABLE_LOGGING
    logging::handle.close();
    shut_down = true;
#endif
}
