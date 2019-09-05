/*
 * init.hpp
 *
 *  Created on: Jul 27, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <stack>

std::stack<void (*)()> &init_stack();
std::stack<void (*)()> &init_stack_early();

class InitRoutine
{
public:
    InitRoutine(void (*func)());
};

class InitRoutineEarly
{
public:
    InitRoutineEarly(void (*func)());
};
