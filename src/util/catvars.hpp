
/*
 *
 *	This is the catvars header file! For more info, read header comment info in .cpp :D
 *
 */

#pragma once

#include <string>	// Strings are easy and hard not to use especially for a value like a catvar where it wont be changing often
#include <vector>	// Makes storage of GUI locations and enum info really hard not to recomend
#include <unordered_map>

#include "../framework/console.hpp"
#include "colors.hpp"

// Makes things easy
typedef const std::vector<std::string> CatEnum;

// Catvar base
class CatVar : public CatComBase {
public:
	CatVar(const CatEnum& _gui_position, std::string _name, std::string _desc_short, std::string _desc_long);
	const CatEnum& gui_position;	// Where to place in menu tree
	std::string name;							// Command name if it needs to be registered into a games console
	std::string desc_short;				// Name in gui
	std::string desc_long;				// Descripton in gui
};
// CatVar varients
class CatVarBool : public CatVar {
public:
	CatVarBool(const CatEnum& _gui_position, std::string _name, bool _defaults, std::string _desc_short, std::string _desc_long = "Unknown");
	inline operator bool() const { return value; }
	inline void operator= (bool in_value) { value = in_value; }
	inline bool operator==(bool in_value) const { return value == in_value; }
	const bool defaults;
	bool value;
	virtual void callback(std::vector<std::string>);
};
class CatVarInt : public CatVar {
public:
	CatVarInt(const CatEnum& _gui_position, std::string _name, int _defaults, std::string _desc_short, std::string _desc_long, int _min, int _max);
	CatVarInt(const CatEnum& _gui_position, std::string _name, int _defaults, std::string _desc_short, std::string _desc_long = "Unknown", int _max = 100)
		: CatVarInt(_gui_position, _name, _defaults, _desc_short, _desc_long, 0, _max) {}
	inline operator int() const { return value; }
	inline void operator= (int in_value) { value = in_value; }
	inline bool operator==(int in_value) const { return value == in_value; }
	const int defaults;
	int value;
	const int min;
	const int max;
	virtual void callback(std::vector<std::string>);
};
class CatVarKey : public CatVarInt {
public:
	CatVarKey(const CatEnum& _gui_position, std::string _name, int _defaults, std::string _desc_short, std::string _desc_long = "Unknown")
		: CatVarInt(_gui_position, _name, _defaults, _desc_short, _desc_long) {}
	virtual void callback(std::vector<std::string>);
};
class CatVarEnum : public CatVarInt {
public:
	CatVarEnum(const CatEnum& _cat_enum, const CatEnum& _gui_position, std::string _name, int _defaults, std::string _desc_short, std::string _desc_long = "Unknown")
		: CatVarInt(_gui_position, _name, _defaults, _desc_short, _desc_long, 0, _cat_enum.size() - 1), cat_enum(_cat_enum) {}
	const CatEnum& cat_enum;
	virtual void callback(std::vector<std::string>);
};
class CatVarFloat : public CatVar {
public:
	CatVarFloat(const CatEnum& _gui_position, std::string _name, float _defaults, std::string _desc_short, std::string _desc_long, float _min, float _max);
	CatVarFloat(const CatEnum& _gui_position, std::string _name, float _defaults, std::string _desc_short, std::string _desc_long = "Unknown", float _max = 100)
		: CatVarFloat(_gui_position, _name, _defaults, _desc_short, _desc_long, 0, _max) {}
	inline operator float() const { return value; }
	inline void operator= (float in_value) { value = in_value; }
	inline bool operator==(float in_value) const { return value == in_value; }
	const float defaults;
	float value;
	const float min;
	const float max;
	virtual void callback(std::vector<std::string>);
};
class CatVarString : public CatVar {
public:
	CatVarString(const CatEnum& _gui_position, std::string _name, std::string _defaults, std::string _desc_short, std::string _desc_long = "Unknown");
	inline operator std::string() const { return value; }
	inline void operator= (const std::string& in_value) { value = in_value; }
	inline bool operator==(const std::string& in_value) const { return value == in_value; }
	const std::string defaults;
	std::string value;	// std::string is perfectly acceptible due to the fact that its mostly static and its extra features are nice
	virtual void callback(std::vector<std::string>);
};
class CatVarColor : public CatVar {
public:
	CatVarColor(const CatEnum& _gui_position, std::string _name, CatVector4 _defaults, std::string _desc_short, std::string _desc_long = "Unknown");
	inline operator CatVector4() const { return value; }
	inline void operator= (CatVector4 in_value) { value = in_value; }
	inline bool operator==(CatVector4 in_value) const { return value == in_value; }
	const CatVector4 defaults;
	CatVector4 value;
	virtual void callback(std::vector<std::string>);
};


// Class to store how the menu is layed out
class CatMenuTree {
public:
	CatMenuTree(std::string string = "") : name(string) {}

	void AddTree(CatVar* cat_var, int recursions = 0);
	std::string name;
	std::vector<CatMenuTree> children;
	std::vector<CatVar*> cat_children;	// Nyaa~ :3
};

// Map to be used for commands
extern std::unordered_map<std::string, CatVar*> CatVarMap;
// Uhh, look in cpp
extern CatMenuTree CatMenuRoot;
