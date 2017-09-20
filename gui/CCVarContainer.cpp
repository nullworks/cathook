/*
 * CCVarContainer.cpp
 *
 *  Created on: Feb 1, 2017
 *      Author: nullifiedcat
 */


#include "CCVarContainer.h"

#include "CSlider.h"
#include "CTextInput.h"
#include "CTextLabel.h"
#include "CKeyInput.h"
#include "CCheckbox.h"
#include "CDropdown.h"

#include "../common.h"
#include "../sdk.h"

CCVarContainer::CCVarContainer(IWidget* parent, CatVar* var) : CBaseContainer("cvc_" + var->name, parent) {
	m_pVar = var;
	m_pInput = 0;
	m_pLabel = new CTextLabel(GetName() + "_desc", this, var->desc_short);
	m_pControl = 0;
	if (var->desc_short.length()) {
		Props()->SetString("tooltip", var->desc_long.c_str());
	}
	bool needsinput = false;
	switch (var->type) {
	case CatVar_t::CV_SWITCH: {// Label, Checkbox
		CCheckbox* cb = new CCheckbox(GetName() + "_control", this, var->GetBool());
		cb->SetCallback([this](CCheckbox*, bool value) {
			m_pVar->SetValue(value);
		});
		m_pControl = cb;
	} break;
	case CatVar_t::CV_ENUM: { // Most difficult thing, dropdown menu
		if (!var->enum_type) {
			logging::Info("VAR %s HAS NO ENUM TYPE!! ERROR!!", var->name.c_str());
			break;
		}
		CDropdown* dd = new CDropdown(GetName() + "_control", this);
		for (int i = var->enum_type->Minimum(); i <= var->enum_type->Maximum(); i++) {
			dd->AddValue(var->enum_type->Name(i));
		}
		dd->SetCallback([this](CDropdown*, int value) {
			m_pVar->SetValue(value);
		});
		dd->Props()->SetInt("offset", var->enum_type->Minimum());
		m_pControl = dd;
	} break;
	case CatVar_t::CV_FLOAT: {
		if (var->restricted) {
			CSlider* sl = new CSlider(GetName() + "_control", this);
			sl->Setup(var->min, var->max);
			sl->SetValue(var->GetFloat());
			m_pControl = sl;
			sl->SetCallback([this](CSlider*, float oldv, float newv) {
				m_pVar->SetValue(newv);
			});
		}
		needsinput = true;
	} break;
	case CatVar_t::CV_INT: {
		if (var->restricted) {
			CSlider* sl = new CSlider(GetName() + "_control", this);
			sl->Setup(var->min, var->max);
			sl->SetStep(1.0f);
			sl->SetValue(var->GetInt());
			sl->SetCallback([this](CSlider*, float oldv, float newv) {
				m_pVar->SetValue(newv);
			});
			m_pControl = sl;
		}
		needsinput = true;
	} break;
	case CatVar_t::CV_KEY: {
		CKeyInput* ki = new CKeyInput(GetName() + "_control", this);
		ki->SetCallback([this](CKeyInput*, ButtonCode_t key) {
			m_pVar->SetValue(key);
		});
		ki->SetSize(60, 18);
		m_pControl = ki;
	} break;
	case CatVar_t::CV_STRING: {
		needsinput = true;
	} break;
	}

	if (needsinput) {
		m_pInput = new CTextInput(GetName() + "_input", this);
		m_pInput->SetValue(std::string(var->GetString()));
		m_pInput->SetCallback([this](CTextInput*, std::string old, std::string newv) {
			if (m_pVar->type == CV_STRING) {
				m_pVar->SetValue(newv);
			} else {
				try {
					char* s = strfmt("%.2f", std::stof(newv));
					std::string str(s);
					delete [] s;
					m_pVar->SetValue(str);
				} catch (...) {

				}
			}
		});
	}

	if (m_pLabel) {
		AddChild(m_pLabel);
	}
	if (m_pControl) {
		AddChild(m_pControl);
	}
	if (m_pInput) {
		m_pInput->SetMaxWidth(10);
		AddChild(m_pInput);
	}
}

void CCVarContainer::MoveChildren() {
	int sy = 0;
	for (auto child : m_children) {
		auto s = child->GetSize();
		if (sy < s.second) sy = s.second;
	}
	auto max = GetMaxSize();
	SetSize(max.first, sy + 4);
	auto labels = m_pLabel->GetSize();
	m_pLabel->SetOffset(2, (sy - labels.second) / 2 + 2);
	int x = max.first - 2;
	if (m_pInput) {
		auto inputs = m_pInput->GetSize();
		x -= inputs.first + 2;
		m_pInput->SetOffset(x + 2, (sy - inputs.second) / 2 + 2);
	}
	if (m_pControl) {
		auto controls = m_pControl->GetSize();
		m_pControl->SetMaxSize(x - 4 - labels.first, sy - 4);
		x -= controls.first + 2;
		m_pControl->SetOffset(x + 2, (sy - controls.second) / 2 + 2);
	}
}

void CCVarContainer::Update() {
	static int updticks = 0;
	updticks++;
	if (m_pVar && !(updticks % 30)) {
		updticks = 1;
		if (m_pInput) {
			if (!m_pInput->IsFocused()) m_pInput->SetValue(m_pVar->GetString());
		}
		if (m_pControl && !m_pControl->IsFocused()) {
			switch (m_pVar->type) {
			case CatVar_t::CV_ENUM: {
				dynamic_cast<CDropdown*>(m_pControl)->SetValue(m_pVar->GetInt());
			} break;
			case CatVar_t::CV_FLOAT:
			case CatVar_t::CV_INT: {
				dynamic_cast<CSlider*>(m_pControl)->SetValue(m_pVar->GetFloat());
			} break;
			case CatVar_t::CV_KEY: {
				dynamic_cast<CKeyInput*>(m_pControl)->SetValue(m_pVar->GetInt());
			} break;
			case CatVar_t::CV_SWITCH: {
				dynamic_cast<CCheckbox*>(m_pControl)->SetValue(m_pVar->GetBool());
			} break;
			}
		}
	}
	CBaseContainer::Update();
}
