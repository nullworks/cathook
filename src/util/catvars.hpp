/*
 * cvwrapper.h
 *
 *  Created on: Dec 17, 2016
 *      Author: nullifiedcat
 */

#ifndef CVWRAPPER_H_
#define CVWRAPPER_H_

#include <string>
#include <vector>
#include <functional>
#include "colors.hpp"

enum CatVar_t {
	CV_SWITCH,
	CV_INT,
	CV_FLOAT,
	CV_STRING,
	CV_ENUM,
	CV_KEY,
	CV_RGBA
};

class CatEnum {
public:
	CatEnum(std::vector<const char*> values);
	const char* Name(int value);
public:
	std::vector<const char*> value_names;
	int min_value;
	int max_value;
	int size;
};

class CatVarBool {
public:
	CatVarBool(CatEnum& gui_position, const char* name, bool defaults, const char* desc_short, const char* desc_long);
	inline operator bool() const 						{ return value_bool; }
	inline void operator =(const bool& value) 			{ value_bool = value; }
	inline bool operator ==(const bool& value) const	{ return value_bool == value; }
public:
	const char* name;			// Command name if it needs to be registered into a games console
	const char* desc_short;	// Name in gui
	const char* desc_long;	// Descripton in gui
	CatEnum* gui_position;		// Where it would be in the gui, not const in the case we want to add the ability to move it later
	const bool defaults;
	bool value_bool;
};

class CatVarInt {
public:
	CatVarInt(CatEnum& gui_position, const char* name, int defaults, const char* desc_short, const char* desc_long);
	CatVarInt(CatEnum& gui_position, const char* name, int defaults, const char* desc_short, const char* desc_long, int max_val);
	CatVarInt(CatEnum& gui_position, const char* name, int defaults, const char* desc_short, const char* desc_long, int min_val, int max_val);
	CatVarInt(CatEnum& cat_enum, CatEnum& gui_position, const char* name, int defaults, const char* desc_short, const char* desc_long);
	CatVarInt(CatEnum& gui_position, const char* name, const char* desc_short, const char* desc_long);	// CatKey, cant have defaults.... No need why I would
	inline operator int() const 						{ return value_int; }
	inline void operator =(const int& value) 			{ value_int = value; }
	inline bool operator ==(const int& value) const		{ return value_int == value; }
public:
	const char* name;			// Command name if it needs to be registered into a games console
	const char* desc_short;	// Name in gui
	const char* desc_long;	// Descripton in gui
	CatEnum* gui_position;
	const int defaults;
	int value_int;
	int max = 100;	// Highest value gui allows
	int min = 0;	// Lowest value gui allows
	CatEnum* cat_enum; // In the case that we use a cat enum, we store it here!
};

class CatVarFloat {
public:
	CatVarFloat(CatEnum& gui_position, const char* name, float defaults, const char* desc_short, const char* desc_long);
	CatVarFloat(CatEnum& gui_position, const char* name, float defaults, const char* desc_short, const char* desc_long, float max_val);
	CatVarFloat(CatEnum& gui_position, const char* name, float defaults, const char* desc_short, const char* desc_long, float min_val, float max_val);
	inline operator float() const 						{ return value_float; }
	inline void operator =(const float& value) 			{ value_float = value; }
	inline bool operator ==(const float& value) const		{ return value_float == value; }
public:
	const char* name;			// Command name if it needs to be registered into a games console
	const char* desc_short;	// Name in gui
	const char* desc_long;	// Descripton in gui
	CatEnum* gui_position;
	const float defaults;
	float value_float;
	float max = 100;	// Highest value gui allows
	float min = 0;	// Lowest value gui allows
};

class CatVarString {
public:
	CatVarString(CatEnum& gui_position, const char* name, const char* defaults, const char* desc_short, const char* desc_long);
	CatVarString(CatEnum& gui_position, const char* name, const char* defaults, const char* desc_short, const char* desc_long, int max_val);
	CatVarString(CatEnum& gui_position, const char* name, const char* defaults, const char* desc_short, const char* desc_long, int min_val, int max_val);
	inline operator std::string() const 							{ return value_string; }
	inline void operator =(const char*& value) 		{ value_string = value; }
	inline bool operator ==(const char*& value) const { return value_string == value; }
public:
	const char* name;			// Command name if it needs to be registered into a games console
	const char* desc_short;	// Name in gui
	const char* desc_long;	// Descripton in gui
	CatEnum* gui_position;
	const char* defaults;
	std::string value_string; // Increase if needed
	float max = 35;	// Do you really want to limit the sizes
	float min = 0;
};

class CatVarColor {
public:
	CatVarColor(CatEnum& gui_position, const char* name, rgba_t defaults, const char* desc_short, const char* desc_long);
	inline operator rgba_t() const 				{ return value_rgba; }
	inline void operator =(const rgba_t& value) { value_rgba = value; }
public:
	const char* name;			// Command name if it needs to be registered into a games console
	const char* desc_short;	// Name in gui
	const char* desc_long;	// Descripton in gui
	CatEnum* gui_position;
	const rgba_t defaults;
	rgba_t value_rgba;
};

// TODO, use virtual functions to link all the catvars together rather than this cruddy ol'object...

class CatVar {
public:
	CatVar(CatVar_t type, CatVarBool* cat_bool, CatVarInt* cat_int, CatVarFloat* cat_float, CatVarString* cat_string, CatVarColor* cat_color);
	//CatVar(CatVar_t type, CatVarBool*, CatVarInt*, CatVarFloat*, CatVarString*, CatVarColor*, std::string name); // In the case we switch to an unordered map
	
		
public:
	const CatVar_t type; 		// How do we use the catvar as
	// Only one should be used and it should change depending on the type.
	CatVarBool* cat_bool;
	CatVarInt* cat_int;
	CatVarFloat* cat_float;
	CatVarString* cat_string;
	CatVarColor* cat_color;
	CatEnum* GetGUIEnum();
	std::string GetName();
};

// Storage for all the catvars! Eventually should use a unordered map
extern std::vector<CatVar*> CatVarList;

#endif /* CVWRAPPER_H_ */
