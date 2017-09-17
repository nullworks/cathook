/*
 * chatstack.cpp
 *
 *  Created on: Jan 24, 2017
 *      Author: nullifiedcat
 */

#include "chatstack.h"
#include "common.h"
#include "sdk.h"

namespace chat_stack {


void Say(const std::string& message, bool team) {
	stack.push({ message, team });
}

void OnCreateMove() {
	if (last_say > g_GlobalVars->curtime) last_say = 0;
	if (g_GlobalVars->curtime - last_say <= CHATSTACK_INTERVAL) return;
	if (!stack.empty()) {
		const msg_t& msg = stack.top();
		stack.pop();
		if (msg.text.size()) {
			//logging::Info("Saying %s %i", msg.text.c_str(), msg.text.size());
			if (msg.team)
				g_IEngine->ServerCmd(format("say_team \"", msg.text.c_str(), '"').c_str());
			else
				g_IEngine->ServerCmd(format("say \"", msg.text.c_str(), '"').c_str());
			last_say = g_GlobalVars->curtime;
		}
	}
}

void Reset() {
	while (!stack.empty()) stack.pop();
	last_say = 0.0f;
}

std::stack<msg_t> stack;
float last_say = 0.0f;

}
