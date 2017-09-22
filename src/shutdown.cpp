/*
 * init.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: nullifiedcat
 */

#include <stack>

#include "logging.h"

#include "shutdown.hpp"


std::stack<void(*)()>& shutdown_stack();

void RunShutdown() {
	logging::Info("Running registered shutdown functions...");
	while (!shutdown_stack().empty()) {
		shutdown_stack().top()();
		shutdown_stack().pop();
	}
	logging::Info("Finished Running shutdown functions...");
}