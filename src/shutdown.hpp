/*
 * init.hpp
 *
 *  Created on: Jul 27, 2017
 *      Author: nullifiedcat
 */

#pragma once
#include <stack>

void RunShutdown();

extern std::stack<void(*)()>& shutdown_stack();


