/*
 * init.hpp
 *
 *  Created on: Jul 27, 2017
 *      Author: nullifiedcat
 */

#pragma once
#include <stack>

void RunShutdown();
typedef void(*funcptr)(void);


void RegisterShutdown(funcptr);