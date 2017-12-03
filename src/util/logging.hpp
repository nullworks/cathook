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
  CatLogger(const char*);
  ~CatLogger();
  void log(const char* fmt, ...);
private:
  FILE* log_handle;
};

// Use this to log
extern CatLogger g_CatLogging;
