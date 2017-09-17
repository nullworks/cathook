/*
 * chatstack.h
 *
 *  Created on: Jan 24, 2017
 *      Author: nullifiedcat
 */

#ifndef CHATSTACK_H_
#define CHATSTACK_H_

#define CHATSTACK_INTERVAL 0.8f

#include "beforecheaders.h"
#include <string>
#include <stack>
#include <functional>
#include "aftercheaders.h"

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

#endif /* CHATSTACK_H_ */
