/*
 * GUI.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#include "../framework/inputmgr.hpp" // So we can update our gui

#include "gui.hpp"

namespace gui { 
	
CatVar gui_visible(CV_SWITCH, "gui_visible", "0", "GUI Active", "GUI switch");
CatVar gui_draw_bounds(CV_SWITCH, "gui_bounds", "0", "Draw Bounds", "Draw GUI elements' bounding boxes");

// We use a base window to contain our root windows and to make managing them simple
CatGUI& g_pGUI() {
	static CatGUI p100;
	return p100;
}

// An init for our catgui
CatGUI::CatGUI() {}

// An update engine for the gui
void CatGUI::Update() {

	// Input handler
	for (int i = 0; i < CATKEY_COUNT; i++) {
		
		// Get whether key is down and whether key state changed from last tick
		bool down = input::stored_pressed[i];
		bool changed = (i == CATKEY_M_WHEEL_DOWN || i == CATKEY_M_WHEEL_UP) ? down : pressed_state[i] != down; // If its a mouse wheel, we dont care about last state
		// Update last state and Tick key was pressed
		pressed_state[i] = down;
		if (changed && down) pressed_frame[i] = frame_count;
		
		// If our key has changed, we send out this keypress to the gui widgets
		if (changed) {
			// Send out a keypress to the gui
			if (down) OnKeyPress(i, false);
			else OnKeyRelease(i);
				
		// If a key hasent changed, We can repeat the key pressed after a period of time
		} else if (down) {
			// Get how long tick was depressed
			int frame = frame_count - pressed_frame[i];
			if (frame && frame > 150) {
				// if frame was pressed for less than 400 ticks, we repeat every 80 ticks, otherwise we repeat every 30
				if ((frame > 400) ? frame % 30 == 0 : frame % 80 == 0) {
					// Send out a keypress to the gui with repeat true
					g_pGUI()->OnKeyPress(i, true);
				}
			}
		}
	}
	// Increse our framecount
	frame_count++;
	
	// Update tick to elements
	Update();
	
	// Draw
	Draw();	
}

// Draw override to prevent drawing onto screen with the root
void GatGUI::Draw() { CBaseContainer::Draw(); }

}
 
