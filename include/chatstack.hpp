/*
 * chatstack.h
 *
 *  Created on: Jan 24, 2017
 *      Author: nullifiedcat
 */

#pragma once

#define CHATSTACK_INTERVAL 0.8f

#include <string>
#include <stack>
#include <functional>

namespace chat_stack
{

struct msg_t
{
    std::string text;
};

void Say(const std::string &message, bool team = false);
void OnCreateMove();
void Reset();

extern std::stack<msg_t> stack;
extern float last_say;
}
