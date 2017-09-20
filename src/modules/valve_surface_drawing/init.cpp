
/*
 *
 *	Welcome to the Valve surface drawing module!
 *	Run the init function below if you wish to use this drawing module.
 *	You must be using a source 2013 game with access to the valve surface interface.
 *		-Onee
 *
 *
 */

#include "../../logging.h"
#include "../../managers/drawmodulemgr.hpp"
#include "draw.hpp"

#include "init.hpp"


namespace modules { namespace valvesurface {

void Init(vgui::ISurface* VSurface) { // To pass surface into the module, it requests it at init
	
	logging::Info("Begin Valve Surface Draw Module Init...");
	g_ISurface = VSurface; // Set the pointer passed from the game module into the draw functions
	
	// Pass the surface functions to the drawmodule manager
	drawmgr::InitLine(Line);
	drawmgr::InitRect(Rect);
	drawmgr::InitRectFilled(RectFilled);
	drawmgr::strings::InitString(String1);
	drawmgr::strings::InitStringLength(StringLength);
	logging::Info("Finish Valve Surface Draw Module Init...");
}
	
}} 
