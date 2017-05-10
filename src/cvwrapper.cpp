/*
 * cvwrapper.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: nullifiedcat
 */

#include "cvwrapper.h"

#include "common.h"
#include "sdk.h"

// Prevent initialization errors.
std::vector<CatVar*>& registrationArray() {
	static std::vector<CatVar*> vector;
	return vector;
}

std::vector<CatCommand*>& commandRegistrationArray() {
	static std::vector<CatCommand*> vector;
	return vector;
}


CatCommand::CatCommand(std::string name, std::string help, FnCommandCallback_t callback)
	: name(name), help(help), callback(callback) {
	commandRegistrationArray().push_back(this);
}

CatCommand::CatCommand(std::string name, std::string help, FnCommandCallbackVoid_t callback)
	: name(name), help(help), callback_void(callback) {
	commandRegistrationArray().push_back(this);
}

void CatCommand::Register() {
	char* name_c = new char[256];
	char* help_c = new char[256];
	if (name.at(0) == '+' || name.at(0) == '-') {
		strncpy(name_c, (name).c_str(), 255);
	} else {
		strncpy(name_c, (CON_PREFIX + name).c_str(), 255);
	}
	strncpy(help_c, help.c_str(), 255);
	if (callback) cmd = new ConCommand(name_c, callback, help_c);
	else if (callback_void) cmd = new ConCommand(name_c, callback_void, help_c);
	else throw std::logic_error("no callback in CatCommand");
	g_ICvar->RegisterConCommand(cmd);
	// name_c and help_c are not freed because ConCommandBase doesn't copy them
}

void RegisterCatCommands() {
	while (!commandRegistrationArray().empty()) {
		CatCommand* cmd = commandRegistrationArray().back();
		cmd->Register();
		commandRegistrationArray().pop_back();
	}
}

CatVar::CatVar(CatVar_t type, std::string name, std::string defaults, std::string desc_short, std::string desc_long)
	: type(type), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long), enum_type(nullptr), restricted(false), callbacks{} {
	min = 0.0f;
	max = 0.0f;
	registrationArray().push_back(this);
}

CatVar::CatVar(CatVar_t type, std::string name, std::string defaults, std::string desc_short, std::string desc_long, float max_val)
	: type(type), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long), enum_type(nullptr), restricted(true), callbacks{} {
	min = 0.0f;
	max = max_val;
	registrationArray().push_back(this);
}

CatVar::CatVar(CatVar_t type, std::string name, std::string defaults, std::string desc_short, std::string desc_long, float min_val, float max_val)
	: type(type), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long), enum_type(nullptr), restricted(true), callbacks{} {
	min = min_val;
	max = max_val;
	registrationArray().push_back(this);
}

CatVar::CatVar(CatEnum& cat_enum, std::string name, std::string defaults, std::string desc_short, std::string desc_long)
	: type(CV_ENUM), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long), enum_type(&cat_enum), restricted(true), callbacks{} {
	min = cat_enum.Minimum();
	max = cat_enum.Maximum();
	registrationArray().push_back(this);
}

void CatVar::OnRegister(CatVar::RegisterCallbackFn fn) {
	if (registered) fn(this);
	else callbacks.push_back(fn);
}

void CatVar::Register() {
	CatVarList().push_back(this);
	convar = CreateConVar(CON_PREFIX + name, defaults, desc_short);
	convar_parent = convar->m_pParent;
	while (!callbacks.empty()) {
		callbacks.back()(this);
		callbacks.pop_back();
	}
	registered = true;
}

void CatVar::InstallChangeCallback(FnChangeCallback_t callback) {
	OnRegister([callback](CatVar* var) {
		var->convar_parent->InstallChangeCallback(callback);
	});
}

void RegisterCatVars() {
	while (registrationArray().size()) {
		CatVar* var = registrationArray().back();
		var->Register();
		registrationArray().pop_back();
	}
}

CatEnum::CatEnum(std::vector<std::string> values, int min) : m_values(values) {
	m_iMin = min;
	m_iMax = min + values.size() - 1;
	m_iLength = values.size();
}

std::string CatEnum::Name(int value) {
	if (value - m_iMin >= 0 && value - m_iMin <= m_iMax) {
		return m_values.at(value - Minimum());
	}
	return "unknown";
}

int CatEnum::Maximum() const {
	return m_iMax;
}

int CatEnum::Minimum() const {
	return m_iMin;
}

std::vector<CatVar*>& CatVarList() {
	static std::vector<CatVar*> object {};
	return object;
}

