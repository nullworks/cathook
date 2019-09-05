/*
 * init.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: nullifiedcat
 */

#include "init.hpp"

std::stack<void (*)()> &init_stack()
{
    static std::stack<void (*)()> stack{};
    return stack;
}

InitRoutine::InitRoutine(void (*func)())
{
    init_stack().push(func);
}

std::stack<void (*)()> &init_stack_early()
{
    static std::stack<void (*)()> stack{};
    return stack;
}

InitRoutineEarly::InitRoutineEarly(void (*func)())
{
    init_stack_early().push(func);
}
