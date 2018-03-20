
/*
 *
 *  Header for the root window
 *
 *
 */

#pragma once

#include "../../framework/input.hpp" // So we can update our gui & know, its included here for CATKEY_COUNT
#include "CBaseParent.hpp"

namespace gui { namespace base {

class CBaseRoot : public CBaseParent {
  //CBaseInfoBox* tooltip_widget; // So we can use tooltips
public:
	CBaseRoot(std::string name = "Root", std::string tooltip="");

	//For redirecting CATKEY_MOUSE_1 to OnMouse
	virtual void OnKeyPress(int key, bool repeat);
	virtual void OnKeyRelease(int key);

	// used to store what frame we are on
	//unsigned long frame_count = 0;
	// Used to store our previous presses and times they were pressed
	//bool pressed_state[CATKEY_COUNT];
	//int  pressed_frame[CATKEY_COUNT];
};

}}
