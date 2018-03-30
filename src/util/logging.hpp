/*
 * logging.h
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include <stdarg.h> // ... arg
#include <stdio.h> // fopen(), fprint(), fputs(), FILE

#include "functions.hpp"

class CatLogger {
public:
  CatLogger(std::string _file_path, bool _ptime = false);
  ~CatLogger();
  void log(const char* fmt, ...); // Use to log with
  CMFunction<void(const char*)> console_tap { [](const char*){return;}};
private:
  FILE* log_handle = nullptr; // Handle used to log to files with
  const bool ptime;
};

// Use this to log
extern CatLogger g_CatLogging;
