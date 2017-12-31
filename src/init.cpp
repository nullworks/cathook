/*
 * init.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: nullifiedcat
 */

#include "init.hpp"

std::stack<void (*)()> &init_stack()
{
    static std::stack<void (*)()> stack;
    return stack;
}

InitRoutine::InitRoutine(void (*func)())
{
    init_stack().push(func);
}
