/*
 * cvwrapper.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: nullifiedcat
 */

#include "logging.h"
#include "catvars.hpp"

// CatBools
CatVarBool::CatVarBool(CatEnum& gui_position, const char* name, bool defaults, const char* desc_short, const char* desc_long)
	: gui_position(&gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	value_bool = defaults;
	CatLogging("Sttuuuuufff: %s", name);
	CatVarList.push_back(new CatVar(CV_SWITCH, this, nullptr, nullptr, nullptr, nullptr));
}
// CatInts
CatVarInt::CatVarInt(CatEnum& gui_position, const char* name, int defaults, const char* desc_short, const char* desc_long)
	: gui_position(&gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	value_int = defaults;
	CatVarList.push_back(new CatVar(CV_INT, nullptr, this, nullptr, nullptr, nullptr));
}
CatVarInt::CatVarInt(CatEnum& gui_position, const char* name, int defaults, const char* desc_short, const char* desc_long, int max_val)
	: gui_position(&gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	value_int = defaults;
	max = max_val;
	CatVarList.push_back(new CatVar(CV_INT, nullptr, this, nullptr, nullptr, nullptr));
}
CatVarInt::CatVarInt(CatEnum& gui_position, const char* name, int defaults, const char* desc_short, const char* desc_long, int min_val, int max_val)
	: gui_position(&gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	value_int = defaults;
	min = min_val;
	max = max_val;
	CatVarList.push_back(new CatVar(CV_INT, nullptr, this, nullptr, nullptr, nullptr));
}
// CatEnums
CatVarInt::CatVarInt(CatEnum& cat_enum, CatEnum& gui_position, const char* name, int defaults, const char* desc_short, const char* desc_long)
	: cat_enum(&cat_enum), gui_position(&gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	value_int = defaults;
	min = cat_enum.min_value;
	max = cat_enum.max_value;
	CatVarList.push_back(new CatVar(CV_ENUM, nullptr, this, nullptr, nullptr, nullptr));
}
// CatKeys
CatVarInt::CatVarInt(CatEnum& gui_position, const char* name, const char* desc_short, const char* desc_long)
	: gui_position(&gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	value_int = 0;
	CatVarList.push_back(new CatVar(CV_INT, nullptr, this, nullptr, nullptr, nullptr));
}
// CatFloats
CatVarFloat::CatVarFloat(CatEnum& gui_position, const char* name, float defaults, const char* desc_short, const char* desc_long)
	: gui_position(&gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	value_float = defaults;
	CatVarList.push_back(new CatVar(CV_STRING, nullptr, nullptr, this, nullptr, nullptr));
}
CatVarFloat::CatVarFloat(CatEnum& gui_position, const char* name, float defaults, const char* desc_short, const char* desc_long, float max_val)
	: gui_position(&gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	value_float = defaults;
	max = max_val;
	CatVarList.push_back(new CatVar(CV_STRING, nullptr, nullptr, this, nullptr, nullptr));
}
CatVarFloat::CatVarFloat(CatEnum& gui_position, const char* name, float defaults, const char* desc_short, const char* desc_long, float min_val, float max_val)
	: gui_position(&gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	value_float = defaults;
	min = min_val;
	max = max_val;
	CatVarList.push_back(new CatVar(CV_STRING, nullptr, nullptr, this, nullptr, nullptr));
}
// CatStrings
CatVarString::CatVarString(CatEnum& gui_position, const char* name, const char* defaults, const char* desc_short, const char* desc_long)
	: gui_position(&gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	value_string = defaults;
	CatVarList.push_back(new CatVar(CV_STRING, nullptr, nullptr, nullptr, this, nullptr));
}
CatVarString::CatVarString(CatEnum& gui_position, const char* name, const char* defaults, const char* desc_short, const char* desc_long, int max_val)
	: gui_position(&gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	value_string = defaults;
	max = max_val;
	CatVarList.push_back(new CatVar(CV_STRING, nullptr, nullptr, nullptr, this, nullptr));
}
CatVarString::CatVarString(CatEnum& gui_position, const char* name, const char* defaults, const char* desc_short, const char* desc_long, int min_val, int max_val)
	: gui_position(&gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	value_string = defaults;
	min = min_val;
	max = max_val;
	CatVarList.push_back(new CatVar(CV_STRING, nullptr, nullptr, nullptr, this, nullptr));
}
// CatColors
CatVarColor::CatVarColor(CatEnum& gui_position, const char* name, rgba_t defaults, const char* desc_short, const char* desc_long)
	: gui_position(&gui_position), name(name), defaults(defaults), desc_short(desc_short), desc_long(desc_long) {
	value_rgba = defaults;
	
	CatVarList.push_back(new CatVar(CV_RGBA, nullptr, nullptr, nullptr, nullptr, this));
}
// Main CatVar
CatVar::CatVar(CatVar_t type, CatVarBool* cat_bool, CatVarInt* cat_int, CatVarFloat* cat_float, CatVarString* cat_string, CatVarColor* cat_color)
	: type(type), cat_bool(cat_bool), cat_int(cat_int), cat_float(cat_float), cat_string(cat_string), cat_color(cat_color) {
}
CatEnum* CatVar::GetGUIEnum() {
	// Make tmp catenum pointer and extract it from the catvar
	CatEnum* tmp;
	switch(this->type) {
	case CV_SWITCH: 
		tmp = this->cat_bool->gui_position;
		break;
	case CV_INT:
		tmp = this->cat_int->gui_position;
		break;
	case CV_ENUM:
		tmp = this->cat_int->gui_position;
		break;
	case CV_KEY:
		tmp = this->cat_int->gui_position;
		break;
	case CV_FLOAT:
		tmp = this->cat_float->gui_position;
		break;
	case CV_STRING:
		tmp = this->cat_string->gui_position;
		break;
	case CV_RGBA:
		tmp = this->cat_color->gui_position;
	}
	return tmp;	
}
std::string CatVar::GetName() {
	// Make tmp catenum pointer and extract it from the catvar
	std::string tmp;
	switch(this->type) {
	case CV_SWITCH: 
		tmp = this->cat_bool->name;
		break;
	case CV_INT:
		tmp = this->cat_int->name;
		break;
	case CV_ENUM:
		tmp = this->cat_int->name;
		break;
	case CV_KEY:
		tmp = this->cat_int->name;
		break;
	case CV_FLOAT:
		tmp = this->cat_float->name;
		break;
	case CV_STRING:
		tmp = this->cat_string->name;
		break;
	case CV_RGBA:
		tmp = this->cat_color->name;
	}
	return tmp;	
}

CatEnum::CatEnum(std::vector<const char*> values) : value_names(values) {
	min_value = 0;
	max_value = values.size();
	size = values.size();
}

const char* CatEnum::Name(int value) {
	if (value >= min_value && value < max_value) {
		return value_names.at(unsigned(value) - unsigned(min_value));
	}
	return "unknown";
}

std::vector<CatVar*> CatVarList;


