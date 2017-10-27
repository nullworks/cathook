/*
 * GUI.h
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "elements/CBaseWindow.h"

namespace gui {
	
class CatGUI : public base::CBaseWindow {
public:
	CatGUI();
	
	// We override draw as we dont want to draw the entire gui space
	virtual void Draw() override;
	
	// used to store what frame we are on
	int frame_count = 0;
	// Used to store our previous presses and times they were pressed
	bool pressed_state[CATKEY_COUNT];
	int  pressed_frame[CATKEY_COUNT];
};

// Use to get the gui
CatGUI& g_pGUI();
 
}