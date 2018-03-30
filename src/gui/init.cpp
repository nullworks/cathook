
/*
 *
 *	This init find initializes all of the gui!
 *	Without this, gui elements may not function.
 *		-Onee
 *
 */

#include "../util/logging.hpp" // debugging
#include "../util/chrono.hpp"
#include "../framework/gameticks.hpp"		// So we can get drawmgr to draw our stuff
#include "../framework/input.hpp"			// For key_pressed & released callbacks

// Stuff to init
#include "menu/menu.hpp"
#include "gui.hpp"
#include "hudstrings/sidestrings.hpp"
//#include "base/examples/test_window.hpp"

#include "init.hpp"

namespace gui {

void Init() {
	// Commented as menu is wip
	/*g_pGui.size = input::GetBounds();
	g_pGui.draw_bounds=false;
	drawmgr.REventDuring([](){g_pGui.Draw();});
	input::key_event.add([](CatKey key, bool newstate){
		//g_CatLogging.log("Key %s %s", input::key_names[key], newstate?"down":"up");
		// Do this in a text input object
		static CatTimer repeat_timer;
		if (newstate)
			repeat_timer.Reset();
		else if (repeat_timer.ResetAfter(std::chrono::seconds(2.5)))
			g_pGui.OnKeyPress(key);
		if (newstate){
			g_pGui.OnKeyPress(key);
		}else{
			g_pGui.OnKeyRelease(key);
		}
		});
	input::bounds_event.add([](std::pair<int,int> bounds){
		g_pGui.size=bounds;
		g_pGui.OnBounds(bounds);
	});
	input::mouse_event.add([](std::pair<int,int> mouse){
		g_pGui.OnMouse(mouse,false);
	});

	// Setup the draw manager to run gui
	//TODO: Make GUI use CatKey instead of int
	menu::Init();*/

	// Other gui elements
	sidestrings::Init();
	//ShowTestWindow();
	//g_pGUI = new CatGUI();
}

}
