/*
 * chatstack.h
 *
 *  Created on: Jan 24, 2017
 *      Author: nullifiedcat
 */

#ifndef CHATSTACK_HPP_
#define CHATSTACK_HPP_

#define CHATSTACK_INTERVAL 0.8f

#include <beforecheaders.hpp>
#include <string>
#include <stack>
#include <functional>
#include <aftercheaders.hpp>

namespace chat_stack {

struct msg_t {
	std::string text;
	bool team;
};

void Say(const std::string& message, bool team = false);
void OnCreateMove();
void Reset();

extern std::stack<msg_t> stack;
extern float last_say;

}

#endif /* CHATSTACK_HPP_ */
