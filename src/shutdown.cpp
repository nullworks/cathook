/*
 * init.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: nullifiedcat
 */

#include <stack>

#include "util/logging.h"

#include "shutdown.hpp"

// TODO, rework to be easier
static std::stack<funcptr> shutdown_stack;

void RunShutdown() {
	CatLogging("Running registered shutdown functions...");
	while (!shutdown_stack.empty()) {
		shutdown_stack.top()();
		shutdown_stack.pop();
	}
	CatLogging("Finished Running shutdown functions...");
}

void RegisterShutdown(funcptr func) {
	shutdown_stack.push(func);
}