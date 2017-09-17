/*
 * init.hpp
 *
 *  Created on: Jul 27, 2017
 *      Author: nullifiedcat
 */

#pragma once

//#include "beforecheaders.h"
#include <stack>
//#include "aftercheaders.h"

std::stack<void(*)()>& init_stack();

class InitRoutine {
public:
	InitRoutine(void(*func)());
};
