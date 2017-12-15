
/*
 * GUI.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#include "gui.hpp"

bool gui_visible = false;

namespace gui {

// We use a base window to contain our root windows and to make managing them simple
CatGUI g_pGUI;

// An init for our catgui
CatGUI::CatGUI() {}

// An update engine for the gui
void CatGUI::Update() {

	// Input handler
	for (int i = 0; i < CATKEY_COUNT; i++) {

		// Get whether key is down and whether key state changed from last tick
		const bool& down = input::pressed_buttons[i];
		const bool changed = (i == CATKEY_M_WHEEL_DOWN || i == CATKEY_M_WHEEL_UP) ? down : pressed_state[i] != down; // If its a mouse wheel, we dont care about last state
		// Update last state and Tick key was pressed
		pressed_state[i] = down;
		if (changed && down) pressed_frame[i] = frame_count;

		// If our key has changed, we send out this keypress to the gui widgets
		if (changed) {
			// Send out a keypress to the gui
			if (down) {
				if (i == CATKEY_MOUSE_1)
					OnMousePress();
				else if (i == CATKEY_INSERT || i == CATKEY_F11)
					gui_visible = !gui_visible;
				else
					OnKeyPress(i, false);

			// For releases of keypresses
			} else {
				if (i == CATKEY_MOUSE_1)
					OnMouseRelease();
				else
					OnKeyRelease(i);
			}

		// If a key hasent changed, We can repeat the key pressed after a period of time
		} else if (down) {
			// Get how long tick was depressed
			const int frame = frame_count - pressed_frame[i];
			if (frame > 150) {
				// if frame was pressed for less than 400 ticks, we repeat every 80 ticks, otherwise we repeat every 30
				if ((frame > 400) ? frame % 30 == 0 : frame % 80 == 0) {
					// Send out a keypress to the gui with repeat true
					OnKeyPress(i, true);
				}
			}
		}
	}

	// Increse our framecount
	frame_count++;

	// Update tick to elements
	CBaseContainer::Update();

	// Draw
	Draw();
}

// This would draw the entire screen so we override
void CatGUI::DrawBounds() { for (auto child : children) if (child->IsVisible()) child->DrawBounds(); }

}
