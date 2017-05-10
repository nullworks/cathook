/*
 * cvwrapper.h
 *
 *  Created on: Dec 17, 2016
 *      Author: nullifiedcat
 */

#ifndef CVWRAPPER_H_
#define CVWRAPPER_H_

class ConVar;

#include "sdk.h"

#include "beforecheaders.h"
#include <string>
#include <vector>
#include <functional>
#include "aftercheaders.h"


//#define CREATE_CV(type, name, defaults, description) \
//	new CatVar(CreateConVar(CON_PREFIX name, defaults, description), type);

//#define CREATE_CV_DESC(type, name, defaults, description, detailed) \
//	new CatVar(CreateConVar(CON_PREFIX name, defaults, description), type, detailed);

enum CatVar_t {
	CV_SWITCH,
	CV_INT,
	CV_FLOAT,
	CV_STRING,
	CV_ENUM,
	CV_KEY
};

class CatEnum {
public:
	CatEnum(std::vector<std::string> values, int min = 0);
	std::string Name(int value);
	int Maximum() const ;
	int Minimum() const ;
	const std::vector<std::string> m_values;
	int m_iMin;
	int m_iMax;
	int m_iLength;
};

class CatCommand {
public:
	CatCommand(std::string name, std::string help, FnCommandCallback_t callback);
	CatCommand(std::string name, std::string help, FnCommandCallbackVoid_t callback);

	void Register();

	const std::string name;
	const std::string help;

	ConCommand* cmd { nullptr };

	FnCommandCallback_t callback { nullptr };
	FnCommandCallbackVoid_t callback_void { nullptr };
};

void RegisterCatCommands();

class CatVar {
public:
	CatVar(CatVar_t type, std::string name, std::string defaults, std::string desc_short, std::string desc_long = "no description");
	CatVar(CatVar_t type, std::string name, std::string defaults, std::string desc_short, std::string desc_long, float max_val);
	CatVar(CatVar_t type, std::string name, std::string defaults, std::string desc_short, std::string desc_long, float min_val, float max_val);
	CatVar(CatEnum& cat_enum, std::string name, std::string defaults, std::string desc_short, std::string desc_long);

	inline operator bool() const { return !!convar_parent->m_nValue; }
	inline operator int() const { return convar_parent->m_nValue; }
	inline operator float() const { return convar_parent->m_fValue; }
	inline void operator =(const int& value) { convar_parent->InternalSetIntValue(value); }
	inline void operator =(const float& value) { convar_parent->InternalSetFloatValue(value); }
	inline bool operator ==(const int& value) const { return convar_parent->m_nValue == value; }
	inline bool operator ==(const float& value) const { return convar_parent->m_fValue == value; }

	void Register();
	typedef std::function<void(CatVar*)> RegisterCallbackFn;
	std::vector<RegisterCallbackFn> callbacks {};
	void OnRegister(RegisterCallbackFn fn);
	void InstallChangeCallback(FnChangeCallback_t callback);

	[[deprecated]]
	inline bool GetBool() const { return this->operator bool();  }
	[[deprecated]]
	inline int GetInt() const { return this->operator int(); }
	[[deprecated]]
	inline float GetFloat() const { return this->operator float(); };
	inline const char* GetString() const { return convar_parent->GetString(); }
	[[deprecated]]
	inline void SetValue(float value) { this->operator =(value); }
	inline void SetValue(std::string value) { convar_parent->SetValue(value.c_str()); }
	[[deprecated]]
	inline void SetValue(int value) { this->operator =(value); }

	bool restricted;
	float max;
	float min;
	bool registered {false};

	const CatVar_t type;
	const std::string name;
	const std::string defaults;
	const std::string desc_short;
	const std::string desc_long;
	CatEnum* enum_type;
	ConVar* convar;
	ConVar* convar_parent;
};

std::vector<CatVar*>& CatVarList();
void RegisterCatVars();

#endif /* CVWRAPPER_H_ */
