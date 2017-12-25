
/*
 *
 *  The guis main window, this handles input, drawing windows, tooltips and more.
 *  This is one of the most important parts of the gui.
 *
 */

#include "CBaseInfoBox.hpp"

#include "CBaseRoot.hpp"

namespace gui { namespace base {

// An init for our catgui
CBaseRoot::CBaseRoot() : CBaseContainer("Root") {
	tooltip_widget = new CBaseInfoBox("tooltip");
	tooltip_widget->position_mode = FLOATING;
	tooltip_widget->max_size = std::make_pair(200, 180);
	AddChild(tooltip_widget);
}

// An update engine for the gui
void CBaseRoot::Update() {

  // Input handler
	for (int i = 0; i < CATKEY_COUNT; i++) {

    // Get whether key is down and whether key state changed from last tick
		const bool& down = input::pressed_buttons[i];
		bool changed = (i == CATKEY_M_WHEEL_DOWN || i == CATKEY_M_WHEEL_UP) ? down : pressed_state[i] != down; // If its a mouse wheel, we dont care about last state
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
  				visible = !visible;
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
  	  int frame = frame_count - pressed_frame[i];
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

	// Tooltip handler
	if (tooltip_widget) { // Sanity check
		// Get and check if we have a tooltip
		auto tooltip_str = GetTooltip();
		if (!tooltip_str.empty()) {
			// Since we have a tooltip to display, we setup the widget to work
			tooltip_widget->Show();
			tooltip_widget->offset = input::mouse;
			tooltip_widget->infostring = tooltip_str;
		} else
			tooltip_widget->Hide();
	}

  // Update tick to elements
  CBaseContainer::Update();

  // Draw
  Draw();
}

// This would draw the entire screen so we override
void CBaseRoot::DrawBounds() { for (auto child : children) if (child->IsVisible()) child->DrawBounds(); }

}}
