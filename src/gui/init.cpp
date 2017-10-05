 
/*
 *
 *	This init find initializes all of the gui!
 *	Without this, gui elements may not function.
 *		-Onee
 *
 */


#include "../framework/drawmgr.hpp"	// So we can get drawmgr to draw our stuff
#include "sidestrings/init.hpp"		// Stuff to init
#include "menu/menu.hpp"
#include "inputmachine.hpp"
#include "drawmachine.hpp"

#include "init.hpp"

namespace gui {
void Init() {
	
	// Setup the draw manager to run gui
	drawmgr::RequestDrawOnDraw(InputMachine);
	drawmgr::RequestDrawOnDraw(DrawMachine);
	
	// Initialize the menu
	drawmgr::RequestDrawOnAfter(menu::Draw);
	
	// Other gui elements
	sidestrings::Init();	
}
}