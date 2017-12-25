/*
 * GUI.h
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "base/CBaseContainer.hpp"
#include "../framework/input.hpp" // So we can update our gui & know, its included here for CATKEY_COUNT

namespace gui { namespace base {

class CBaseRoot: public CBaseContainer {
public:
	CBaseRoot();

	void DisplayTooltip();
	void MoveChildren(){};
	void Update();
	void DrawBounds();

	// used to store what frame we are on
	unsigned long frame_count = 0;
	// Used to store our previous presses and times they were pressed
	bool pressed_state[CATKEY_COUNT];
	int  pressed_frame[CATKEY_COUNT];
};

}}
