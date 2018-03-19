
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

// Stuff to init
//#include "gui.hpp"
#include "menu/menu.hpp"
#include "hudstrings/sidestrings.hpp"
#include "base/examples/test_window.hpp"

#include "init.hpp"

namespace gui {

void Init() {

	// Setup the draw manager to run gui
	//TODO: Make GUI use CatKey instead of int
	drawmgr.REventDuring([](){g_pGui.Update();});
	input::key_event.add([](CatKey key, bool newstate){
		if (newstate){
			g_pGui.OnKeyPress(key);
		}else{
			g_pGui.OnKeyRelease(key);
		}
		});
	input::bounds_event.add([](std::pair<int,int> bounds){
		g_pGui.minmax_size=bounds;
	});
	input::mouse_event.add([](std::pair<int,int> mouse){
		//TODO: gui - mouse
	});
	menu::Init();

	// Other gui elements
	sidestrings::Init();
	ShowTestWindow();
	//g_pGUI = new CatGUI();
	//drawmgr.REventDuring([](){g_pGui.Update();});
}

}
