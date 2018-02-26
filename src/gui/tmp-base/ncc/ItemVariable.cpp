/*
 * ItemVariable.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#include "ItemVariable.hpp"
#include "Item.hpp"
#include "../../../util/catvars.hpp"
#include "../GUI.h"
namespace menu { namespace ncc {

ItemVariable::ItemVariable(CatVar& variable) : Item("ncc_item_variable_" + variable.name), catvar(variable) {

}

void ItemVariable::Update() {
	Item::Update();
	if (catvar.desc_long.length() && IsHovered() && catvar.desc_long != "no description")
		ShowTooltip(catvar.desc_long);
}

enum CatVar_t {
	CV_SWITCH,
	CV_ENUM,
	CV_FLOAT,
	CV_INT,
	CV_KEY,
	CV_STRING
};
static CatVar_t GetCatVarType(CatVar* in) {
	if (auto tmp = dynamic_cast<CatVarBool*>(in))
		return CV_SWITCH;
	if (auto tmp = dynamic_cast<CatVarFloat*>(in))
		return CV_FLOAT;
	if (auto tmp = dynamic_cast<CatVarKey*>(in))
		return CV_KEY;
	if (auto tmp = dynamic_cast<CatVarString*>(in))
		return CV_STRING;
	if (auto tmp = dynamic_cast<CatVarEnum*>(in))
		return CV_ENUM;
	if (auto tmp = dynamic_cast<CatVarInt*>(in))
		return CV_INT;
	return (CatVar_t)0;
}

void ItemVariable::Change(float amount) {
	if (!amount) return;
	switch (GetCatVarType(&catvar)) {
	case CV_SWITCH: {
		(CatVarBool&)catvar = !(CatVarBool&)catvar;
	} break;
	case CV_ENUM:
	case CV_INT: {
		(CatVarInt&)catvar = (CatVarInt&)catvar + (int)amount;
		if ((CatVarInt&)catvar > ((CatVarInt&)catvar).max) (CatVarInt&)catvar = ((CatVarInt&)catvar).max;
		if ((CatVarInt&)catvar < ((CatVarInt&)catvar).min) (CatVarInt&)catvar = ((CatVarInt&)catvar).min;
	} break;
	case CV_FLOAT: {
		(CatVarFloat&)catvar = (CatVarFloat&)catvar + amount;
		if ((CatVarFloat&)catvar > ((CatVarFloat&)catvar).max) (CatVarFloat&)catvar = ((CatVarFloat&)catvar).max;
		if ((CatVarFloat&)catvar < ((CatVarFloat&)catvar).min) (CatVarFloat&)catvar = ((CatVarFloat&)catvar).min;
	} break;
	}
}

bool ItemVariable::ConsumesKey(int key) {
	if (capturing) return true;
	if (key == CATKEY_M_WHEEL_DOWN || key == CATKEY_M_WHEEL_UP || key == CATKEY_MOUSE_1) return true;
	return false;
}

void ItemVariable::OnMousePress() {
	auto type = GetCatVarType(&catvar);
	if (type == CV_KEY)
		capturing = true;
	if (type == CV_SWITCH)
		(CatVarBool&)catvar = !(CatVarBool&)catvar;
}

void ItemVariable::OnFocusLose() {
	capturing = false;
}

void ItemVariable::OnKeyPress(int key, bool repeat) {
	if (capturing) {
		(CatVarInt&)catvar = (int)key;
		capturing = false;
		return;
	}

	float change = 0.0f;

	switch (GetCatVarType(&catvar)) {
	case CV_ENUM:
	case CV_SWITCH:
		change = 1.0f; break;
	case CV_INT: {
		change = float(((CatVarInt&)catvar).max - ((CatVarInt&)catvar).min) / 50.0f;
		break;
	}
	case CV_FLOAT: {
		change = float(((CatVarFloat&)catvar).max - ((CatVarFloat&)catvar).min) / 50.0f;
	}
	}

	if (change < 1.0f && GetCatVarType(&catvar) == CV_INT) change = 1.0f;

	if ((GetCatVarType(&catvar) == CV_SWITCH && key == CATKEY_MOUSE_1) || key == CATKEY_M_WHEEL_UP) {
		Change(change);
	} else if (key == CATKEY_M_WHEEL_DOWN) {
		Change(-change);
	}
}

void ItemVariable::Draw(int x, int y) {
	Item::Draw(x, y);
	std::string val = "[UNDEFINED]";
	switch (GetCatVarType(&catvar)) {
	case CV_SWITCH:
	case CV_INT:
	case CV_FLOAT:
	case CV_ENUM:
	case CV_STRING:
		val = catvar.GetValue();
	case CV_KEY: {
		if (capturing) {
			val = "[PRESS A KEY]";
		} else {
			if ((CatVarKey&)catvar) {
				val = catvar.GetValue();
			} else {
				val = "[CLICK TO SET]";
			}
		}
	} break;
	}
	draw::String((catvar.desc_short + ": " + val).c_str(), x + 2, y, draw::default_font, draw::default_font_size, colors::white) ;
}

}}
