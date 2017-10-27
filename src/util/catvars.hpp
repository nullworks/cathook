/*
 * cvwrapper.h
 *
 *  Created on: Dec 17, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "colors.hpp"

// Storage for all the catvars!
extern std::unordered_map<std::string, CatVar&> CatVarList;

// Makes things easy
typedef const std::vector<const char*> CatEnum;

enum {
	CV_SWITCH,
	CV_INT,
	CV_KEY,
	CV_FLOAT,
	CV_ENUM,
	CV_STRING,
	CV_COLOR
}

class CatVar {
public:
	// bool, int, float, key, string
	CatVar(const int& _type, const CatEnum& _gui_position, const char* _name, const char* _defaults, const char* _desc_short, const char* _desc_long = "Unknown", const int& _max = 100)
		: type(_type), gui_position(_gui_position), name(_name), default_s(_defaults), desc_short(_desc_short), desc_long(_desc_long), max(_max) {
			SetDefault();
			CatVarList.insert({name, this}); 
	};
	CatVar(const int& _type, const CatEnum& _gui_position, const char* _name, const char* _defaults, const char* _desc_short, const char* _desc_long, const int& _min, const int& _max)
		: type(_type), gui_position(_gui_position), name(_name), default_s(_defaults), desc_short(_desc_short), desc_long(_desc_long), min(_min), max(_max) {
			SetDefault();
			CatVarList.insert({name, this}); 
	};
	// Color
	CatVar(const int& _type, const CatEnum& _gui_position, const char* _name, const CatVector4& _defaults, const char* _desc_short, const char* _desc_long = "Unknown")
		: type(_type), gui_position(_gui_position), name(_name), default_v(_defaults), desc_short(_desc_short), desc_long(_desc_long) { 
			SetDefault();
			CatVarList.insert({name, this});
	};
	// CatEnum
	CatVar(const CatEnum& _cat_enum, const CatEnum& _gui_position, const char* _name, const char* _defaults, const char* _desc_short, const char* _desc_long = "Unknown")
		: cat_enum(_cat_enum), type(CV_ENUM), gui_position(_gui_position), name(_name), default_v(_defaults), desc_short(_desc_short), desc_long(_desc_long) { 
			SetDefault();
			CatVarList.insert({name, this});
	};
	// Operators
	inline operator bool() const 		{ return value_i; }
	inline operator int() const 		{ return value_i; }
	inline operator float() const 		{ return value_v.x; }
	inline operator std::string() const	{ return value_s; }
	inline operator CatVector4() const 	{ return value_v; }
	inline void operator  =(const bool& in_value) 			{ value_i = in_value; }
	inline void operator  =(const int& in_value) 			{ value_i = in_value; }
	inline void operator  =(const float& in_value) 			{ value_v.x = in_value; }
	inline void operator  =(const std::string& in_value) 	{ value_s = in_value; }
	inline void operator  =(const CatVector4& in_value) 	{ value_v = in_value; }
	inline bool operator ==(const bool& in_value) const			{ return bool(value_i) == in_value; }
	inline bool operator ==(const int& in_value) const			{ return value_i == in_value; }
	inline bool operator ==(const float& in_value) const		{ return value_v.x == in_value; }
	inline bool operator ==(const std::string& in_value) const	{ return value_s == in_value; }
	inline bool operator ==(const CatVector4& in_value) const	{ return value_v == in_value; }

	// Catvar info
	const int type;
	const CatEnum& gui_position;
	const char* name;		// Command name if it needs to be registered into a games console
	const char* desc_short;	// Name in gui
	const char* desc_long;	// Descripton in gui
	const CatEnum* cat_enum = nullptr;
	// Defaults
	const CatVector4 default_v;
	const char* default_s = "";
	// Min max
	const int min = 0;
	const int max = 32;
	
protected:
	void SetDefault() {
		switch(type) {
		case CV_SWITCH:
		case CV_INT:
		case CV_KEY:
		case CV_ENUM:
			value_i = atoi(default_s); break;
		case CV_FLOAT:
			value_v.x = atof(default_s); break;
		case CV_COLOR:
			value_v = default_v; break;
		default:
			value_s = default_s;
		}
	}
    int value_i;
	CatVector4 value_v;
	std::string value_s;
};



