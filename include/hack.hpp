/*
 * hack.h
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#pragma once

class IHack;
class CUserCmd;
class CViewSetup;
class bf_read;
class ConCommand;
class CCommand;

#include <stack>
#include <string>
#include <mutex>

namespace hack
{

extern std::mutex command_stack_mutex;
std::stack<std::string> &command_stack();
void ExecuteCommand(const std::string command);

extern bool shutdown;
extern bool initialized;

const std::string &GetVersion();
const std::string &GetType();
void Initialize();
void Think();
void Shutdown();
} // namespace hack
