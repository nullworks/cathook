/*
 * CBaseWindow.h
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "CBaseContainer.hpp"

namespace gui { namespace base {

class CBaseWindow : public CBaseContainer {
public:
	// Constructors & Deconstructors
	CBaseWindow(const char* _name = "unnamed", IWidget* _parent = nullptr) : CBaseContainer(name, parent) {
		position_mode = FLOATING;
	}
	virtual ~CBaseWindow() {};

	// General functions
	virtual void Draw() override;

	// User Input
	virtual void OnFocusGain() override;
	virtual void OnFocusLose() override;

	// Child related util
	virtual void MoveChildren() override;
};

}}
