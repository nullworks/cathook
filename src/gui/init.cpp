 
/*
 *
 *	This init find initializes all of the gui!
 *	Without this, gui elements may not function.
 *		-Onee
 *
 */


#include "../managers/drawmgr.hpp"	// So we can get drawmgr to draw our stuff
#include "sidestrings/init.hpp"		// Stuff to init
#include "menu/menu.hpp"
#include "inputmachine.hpp"
#include "drawmachine.hpp"

#include "init.hpp"

namespace gui {
void Init() {
	
	// Setup the draw manager to run gui
	drawmgr::RequestDrawOnDraw(menu::Construct);
	drawmgr::RequestDrawOnDraw(InputMachine);
	
	// Setup the draw manager to run gui
	drawmgr::RequestDrawOnAfter(DrawMachine);
	
	// Other gui elements
	sidestrings::Init();	
}
}