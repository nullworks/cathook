/*
 * logging.h
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include <stdarg.h> // ... arg
#include <stdio.h> // fopen(), fprint(), fputs(), FILE
#include <string>

class CatLogger {
public:
  CatLogger(const char*  _file_path, bool _ptime = false);
  ~CatLogger();
  void log(const char* fmt, ...); // Use to log with
private:
  FILE* log_handle = nullptr; // Handle used to log to files with
  const char* file_path; // Path to log file, needs to be fixed
  const bool ptime;
};

// Use this to log
extern CatLogger g_CatLogging;
