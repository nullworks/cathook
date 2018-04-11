
/*
 *
 *	This is the catvars header file! For more info, read header comment info in .cpp :D
 *
 */

#pragma once

#include <string>	// Strings are easy and hard not to use especially for a value like a catvar where it wont be changing often
#include <vector>	// Makes storage of GUI locations and enum info really hard not to recomend
#include <unordered_map>

#include "../framework/input.hpp" // CatVarKey uses it to look for catkeys as well as its Depressed Function
#include "../framework/console.hpp"
#include "colors.hpp"

// Makes things easy
typedef const std::vector<std::string> CatEnum;

// Catvar base
class CatVar : public CatComBase {
public:
	CatVar(const CatEnum& _gui_position, const char* _name, const char* _desc_short, const char* _desc_long);
	~CatVar();
	const CatEnum& gui_position;	// Where to place in menu tree
	const std::string name;				// Command name if it needs to be registered into a games console
	const char* desc_short;				// Name in gui
	const char* desc_long;				// Descripton in gui
	virtual std::string GetValue() = 0; // Used by cfg mgr and gui
	static std::vector<CatVar*> List;
};
// CatVar varients
class CatVarBool : public CatVar {
public:
	CatVarBool(const CatEnum& _gui_position, const char* _name, bool _defaults, const char* _desc_short, const char* _desc_long = "Unknown");
	inline operator bool() const { return value; }
	inline void operator= (bool in_value) { value = in_value; }
	inline bool operator==(bool in_value) const { return value == in_value; }
	const bool defaults;
	bool value;
	virtual void callback(std::vector<std::string>);
	virtual std::string GetValue();
};
class CatVarInt : public CatVar {
public:
	CatVarInt(const CatEnum& _gui_position, const char* _name, int _defaults, const char* _desc_short, const char* _desc_long, int _min, int _max);
	CatVarInt(const CatEnum& _gui_position, const char* _name, int _defaults, const char* _desc_short, const char* _desc_long = "Unknown", int _max = 100)
		: CatVarInt(_gui_position, _name, _defaults, _desc_short, _desc_long, 0, _max) {}
	inline operator int() const { return value; }
	inline void operator= (int in_value) { value = in_value; }
	inline bool operator==(int in_value) const { return value == in_value; }
	const int defaults;
	int value;
	const int min;
	const int max;
	virtual void callback(std::vector<std::string>);
	virtual std::string GetValue();
};
class CatVarKey : public CatVar {
public:
	CatVarKey(const CatEnum& _gui_position, const char* _name, CatKey _defaults, const char* _desc_short, const char* _desc_long = "Unknown")
		: CatVar(_gui_position, _name, _desc_short, _desc_long), value(_defaults), defaults(_defaults) {}
	CatKey value;
	const CatKey defaults;
	inline operator CatKey() const { return value; }
	inline void operator= (CatKey in_value) { value = in_value; }
	inline bool operator==(CatKey in_value) const { return value == in_value; }
	inline bool Depressed() const { return (*this) ? input::GetKey(this->value) : true;}
	virtual void callback(std::vector<std::string>);
	virtual std::string GetValue();
};
class CatVarEnum : public CatVarInt {
public:
	CatVarEnum(const CatEnum& _gui_position, const CatEnum& _cat_enum, const char* _name, int _defaults, const char* _desc_short, const char* _desc_long = "Unknown")
		: CatVarInt(_gui_position, _name, _defaults, _desc_short, _desc_long, 0, _cat_enum.size() - 1), cat_enum(_cat_enum) {}
	const CatEnum& cat_enum;
	virtual void callback(std::vector<std::string>);
	virtual std::string GetValue();
};
class CatVarFloat : public CatVar {
public:
	CatVarFloat(const CatEnum& _gui_position, const char* _name, float _defaults, const char* _desc_short, const char* _desc_long, float _min, float _max);
	CatVarFloat(const CatEnum& _gui_position, const char* _name, float _defaults, const char* _desc_short, const char* _desc_long = "Unknown", float _max = 100)
		: CatVarFloat(_gui_position, _name, _defaults, _desc_short, _desc_long, 0, _max) {}
	inline operator float() const { return value; }
	inline void operator= (float in_value) { value = in_value; }
	inline bool operator==(float in_value) const { return value == in_value; }
	const float defaults;
	float value;
	const float min;
	const float max;
	virtual void callback(std::vector<std::string>);
	virtual std::string GetValue();
};
class CatVarString : public CatVar {
public:
	CatVarString(const CatEnum& _gui_position, const char* _name, const char* _defaults, const char* _desc_short, const char* _desc_long = "Unknown");
	inline operator std::string() const { return value; }
	inline void operator= (const std::string& in_value) { value = in_value; }
	inline bool operator==(const std::string& in_value) const { return value == in_value; }
	const std::string defaults;
	std::string value;	// std::string is perfectly acceptible due to the fact that its mostly static and its extra features are nice
	virtual void callback(std::vector<std::string>);
	virtual std::string GetValue();
};
class CatVarColor : public CatVar {
public:
	CatVarColor(const CatEnum& _gui_position, const char* _name, CatColor _defaults, const char* _desc_short, const char* _desc_long = "Unknown");
	inline operator CatColor() const { return rainbow ? colors::RainbowCurrent() : value; }
	inline void operator= (CatColor in_value) { value = in_value; }
	inline bool operator==(CatColor in_value) const { return value == in_value; }
	const CatColor defaults;
	CatColor value;
	bool rainbow = false; // Change to set color to rainbow
	virtual void callback(std::vector<std::string>);
	virtual std::string GetValue();
};

// Class to store how the menu is layed out
class CatMenuTree {
public:
	CatMenuTree(std::string string = "") : name(string) {}

	void AddTree(CatVar* cat_var, size_t recursions = 0);
	std::string name;
	std::vector<CatMenuTree> children;
	std::vector<CatVar*> cat_children;	// Nyaa~ :3
};

// Uhh, look in cpp
extern CatMenuTree CatMenuRoot;
