/*
 * hack.h
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#ifndef HACK_H_
#define HACK_H_

/*class IHack;
class CUserCmd;
class CViewSetup;
class bf_read;
class ConCommand;
class CCommand;*/

#include <stack>
//#include <string>
#include <mutex>


namespace hack {
	
/*extern std::mutex command_stack_mutex;
std::stack<std::string>& command_stack();
void ExecuteCommand(const std::string command);*/

extern bool shutdown;

/*const std::string& GetVersion();
const std::string& GetType();*/
void Initialize();
void Think();
void Shutdown();

/*void CC_Cat(const CCommand& args);
extern ConCommand* c_Cat;*/

}

#endif /* HACK_H_ */
