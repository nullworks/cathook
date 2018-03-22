
/*
 *
 *	This init find initializes all of the gui!
 *	Without this, gui elements may not function.
 *		-Onee
 *
 */

// Stuff to init with
#include "../framework/gameticks.hpp"		// So we can get drawmgr to draw our stuff
#include "../framework/input.hpp"			// For key_pressed & released callbacks
#include "../util/logging.hpp"

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
		if (newstate){
			g_pGui.OnKeyPress(key, false);
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
