/*
 * cvwrapper.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: nullifiedcat
 */

#include "catvars.hpp"

CatVar::CatVar(CatVar_t type, CatVarEnum gui_position, std::string name, std::string defaults, std::string desc_short, std::string desc_long)
	: type(type), gui_position(gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	min = 0.0f;
	max = 0.0f;
	CatVarList().push_back(this);
}

CatVar::CatVar(CatVar_t type, CatVarEnum gui_position, std::string name, std::string defaults, std::string desc_short, std::string desc_long, float max_val)
	: type(type), gui_position(gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	min = 0.0f;
	max = max_val;
	CatVarList().push_back(this);
}

CatVar::CatVar(CatVar_t type, CatVarEnum gui_position, std::string name, std::string defaults, std::string desc_short, std::string desc_long, float min_val, float max_val)
	: type(type), gui_position(gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	min = min_val;
	max = max_val;
	CatVarList().push_back(this);
}

CatVar::CatVar(CatVarEnum& cat_enum, CatVarEnum gui_position, std::string name, std::string defaults, std::string desc_short, std::string desc_long)
	: type(CV_ENUM), gui_position(gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	min = cat_enum.min_value;
	max = cat_enum.max_value;
	CatVarList().push_back(this);
}

CatVarEnum::CatVarEnum(std::vector<std::string> values) : value_names(values) {
	min_value = 0;
	max_value = values.size() - 1;
	size = values.size();
}

std::string CatVarEnum::Name(int value) {
	if (value >= min_value && value < max_value) {
		return value_names.at(unsigned(value) - unsigned(min_value));
	}
	return "unknown";
}

std::vector<CatVar*>& CatVarList() {
	static std::vector<CatVar*> object {};
	return object;
}


