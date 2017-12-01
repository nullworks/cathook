
/*
 *
 *	This is the catvars header file! For more info, read header comment info in .cpp :D
 *
 */

#pragma once

#include <string>	// Strings are easy and hard not to use especially for a value like a catvar where it wont be changing often
#include <vector>	// Makes storage of GUI locations and enum info really hard not to recomend
#include <unordered_map>	// We use this to store our map of commands

#include "../util/colors.hpp"

// Makes things easy
typedef std::vector<const char*> CatEnum;

// Catvar base
class CatVar {
public:
	CatVar(const CatEnum& _gui_position, const char* _name, const char* _desc_short, const char* _desc_long);
	const CatEnum& gui_position;	// Where to place in menu tree
	const char* name;							// Command name if it needs to be registered into a games console
	const char* desc_short;				// Name in gui
	const char* desc_long;				// Descripton in gui
};
// CatVar varients
class CatVarBool : public CatVar {
public:
	CatVarBool(const CatEnum& _gui_position, const char* _name, const bool& _defaults, const char* _desc_short, const char* _desc_long = "Unknown");
	inline operator bool() const { return value; }
	inline void operator= (const bool& in_value) { value = in_value; }
	inline bool operator==(const bool& in_value) const { return value == in_value; }
	const bool& defaults;
	bool value;
};
class CatVarInt : public CatVar {
public:
	CatVarInt(const CatEnum& _gui_position, const char* _name, const int& _defaults, const char* _desc_short, const char* _desc_long = "Unknown", const int& _max = 100);
	CatVarInt(const CatEnum& _gui_position, const char* _name, const int& _defaults, const char* _desc_short, const char* _desc_long, const int& _min, const int& _max);
	inline operator int() const { return value; }
	inline void operator= (const int& in_value) { value = in_value; }
	inline bool operator==(const int& in_value) const { return value == in_value; }
	const int& defaults;
	int value;
	const int& min;
	const int& max;
};
class CatVarFloat : public CatVar {
public:
	CatVarFloat(const CatEnum& _gui_position, const char* _name, const float& _defaults, const char* _desc_short, const char* _desc_long = "Unknown", const float& _max = 100);
	CatVarFloat(const CatEnum& _gui_position, const char* _name, const float& _defaults, const char* _desc_short, const char* _desc_long, const float& _min, const float& _max);
	inline operator float() const { return value; }
	inline void operator= (const float& in_value) { value = in_value; }
	inline bool operator==(const float& in_value) const { return value == in_value; }
	const float& defaults;
	float value;
	const float& min;
	const float& max;
};
class CatVarKey : public CatVar {
public:
	CatVarKey(const CatEnum& _gui_position, const char* _name, const int& _defaults, const char* _desc_short, const char* _desc_long = "Unknown");
	inline operator int() const { return value; }
	inline void operator= (const int& in_value) { value = in_value; }
	inline bool operator==(const int& in_value) const { return value == in_value; }
	const int& defaults;
	int value;
};
class CatVarString : public CatVar {
public:
	CatVarString(const CatEnum& _gui_position, const char* _name, const char* _defaults, const char* _desc_short, const char* _desc_long = "Unknown");
	inline operator std::string() const { return value; }
	inline void operator= (const std::string& in_value) { value = in_value; }
	inline bool operator==(const std::string& in_value) const { return value == in_value; }
	const char* defaults;
	std::string value;	// std::string is perfectly acceptible due to the fact that its mostly static and its extra features are nice
};
class CatVarColor : public CatVar {
public:
	CatVarColor(const CatEnum& _gui_position, const char* _name, const CatVector4& _defaults, const char* _desc_short, const char* _desc_long = "Unknown");
	inline operator CatVector4() const { return value; }
	inline void operator= (const CatVector4& in_value) { value = in_value; }
	inline bool operator==(const CatVector4& in_value) const { return value == in_value; }
	const CatVector4& defaults;
	CatVector4 value;
};
class CatVarEnum : public CatVar {
public:
	CatVarEnum(const CatEnum& _cat_enum, const CatEnum& _gui_position, const char* _name, const int& _defaults, const char* _desc_short, const char* _desc_long = "Unknown");
	inline operator int() const { return value; }
	inline void operator= (const int& in_value) { value = in_value; }
	inline bool operator==(const int& in_value) const { return value == in_value; }
	const int& defaults;
	int value;
	const CatEnum& cat_enum;
	const int min;
	const int max;
};

// Class to store how the menu is layed out
class CatMenuTree {
public:
	CatMenuTree(const char* string = "") : name(string) {}

	void AddTree(CatVar* cat_var, int recursions = 0);
	const char* name;
	std::vector<CatMenuTree> children;
	std::vector<CatVar*> cat_children;	// Nyaa~ :3
};

// Map to be used for commands, TODO, move to own file and make CatCommands again with an interpeter
extern std::unordered_map<std::string, CatVar*> CatCommandMap;
// Uhh, look in cpp
extern CatMenuTree CatMenuRoot;
