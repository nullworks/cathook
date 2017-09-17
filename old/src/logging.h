/*
 * logging.h
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include <stdio.h>

typedef void(fn_Msg_t)(const char* msg, va_list);

#ifdef __cplusplus
namespace logging {
#endif

extern FILE* handle;

void Initialize();
void Shutdown();
void Info(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* LOGGING_H_ */
