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
	CBaseWindow(const char*);
	virtual ~CBaseWindow() {};

	// General functions
	virtual void Draw() override;

	// User Input
	virtual bool TryFocusGain() override;
};

}}
