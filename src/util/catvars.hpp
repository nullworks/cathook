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
	CV_COLOR
};

class CatVarEnum {
public:
	CatVarEnum(std::vector<std::string> values);
	std::string Name(int value);
public:
	std::vector<std::string> value_names;
	int min_value;
	int max_value;
	int size;
};

class CatVar {
public:
	CatVar(CatVar_t type, CatVarEnum gui_position, std::string name, std::string defaults, std::string desc_short, std::string desc_long);
	CatVar(CatVar_t type, CatVarEnum gui_position, std::string name, std::string defaults, std::string desc_short, std::string desc_long, float max_val);
	CatVar(CatVar_t type, CatVarEnum gui_position, std::string name, std::string defaults, std::string desc_short, std::string desc_long, float min_val, float max_val);
	CatVar(CatVarEnum& cat_enum, CatVarEnum gui_position, std::string name, std::string defaults, std::string desc_short, std::string desc_long);

public:
	const CatVar_t type; 		// How do we use the catvar as
	CatVarEnum gui_position;	// Where it would be in the gui, not const in the case we want to add the ability to move it later
	const std::string name;		// Command name if it needs to be registered into a games console
	const std::string defaults;	// Defaut value?
	const std::string desc_short;// Name in gui
	const std::string desc_long;// Descripton in gui
	float max;					// Highest value gui allows
	float min;					// Lowest value gui allows
	
	// Storage vars for the guis values, only one gets used sadly but thats okay!
	bool value_bool;
	int value_int;
	float value_float;
	std::string value_string;
	rgba_t value_rgba;
	
	// In the case that we use a cat enum, we store it here!
	CatVarEnum* cat_enum;
};

// Storage for all the catvars!
std::vector<CatVar*>& CatVarList();

#endif /* CVWRAPPER_H_ */
