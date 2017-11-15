/*
 * CBaseWindow.h
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "CBaseContainer.hpp"

class CBaseWindow : public CBaseContainer {
public:
	// Constructors & Deconstructors
	inline CBaseWindow(std::string name = "unnamed", IWidget* parent = nullptr) : CBaseContainer(name, parent) {}
	inline virtual ~CBaseWindow() {};

	// General functions
	virtual void Draw() override;

	// User Input
	virtual void OnFocusGain() override;
	virtual void OnFocusLose() override;

	// Child related util
	virtual void MoveChildren() override;
};
