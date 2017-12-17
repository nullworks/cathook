
/*
 *
 *	This init find initializes all of the gui!
 *	Without this, gui elements may not function.
 *		-Onee
 *
 */

// Stuff to init with
#include "../framework/gameticks.hpp"		// So we can get drawmgr to draw our stuff

// Stuff to init
#include "gui.hpp"
#include "menu/menu.hpp"
#include "hudstrings/sidestrings.hpp"

#include "init.hpp"

namespace gui {

void Init() {

	// Setup the draw manager to run gui
	drawmgr_on([](){g_pGUI.Update();});

	menu::Init();

	// Other gui elements
	sidestrings::Init();
}

}
