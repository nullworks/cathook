/*
 * GUI.h
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "base/CBaseContainer.hpp"
#include "../framework/input.hpp" // So we can update our gui & know

namespace gui {

class CatGUI : public base::CBaseContainer {
public:
	CatGUI();

	virtual void Update() override;
	virtual void DrawBounds() override;

	// used to store what frame we are on
	int frame_count = 0;
	// Used to store our previous presses and times they were pressed
	bool pressed_state[CATKEY_COUNT];
	int  pressed_frame[CATKEY_COUNT];
};

// Use to get the gui
extern CatGUI g_pGUI;

void Update();

}
