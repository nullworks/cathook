/*
 * logging.h
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include <stdio.h>

class CatLogger {
public:
  CatLogger(const char* file_path, bool _ptime = false);
  ~CatLogger();
  const bool ptime;
  void log(const char* fmt, ...);
private:
  FILE* log_handle;
};

// Use this to log
extern CatLogger g_CatLogging;
