
/*
 *
 *	Welcome to the Valve surface drawing module!
 *	Run the init function below if you wish to use this drawing module.
 *	You must be using a source 2013 game with access to the valve surface interface.
 *		-Onee
 *
 *
 */

#include "../../managers/drawmodulemgr.hpp"
#include "draw.hpp"

#include "init.hpp"

// So that the modularity funcs work
typedef void(*fn_name_int)(int, ...); 
typedef void(*fn_name_const_char)(const char*, ...);
typedef bool(*fn_name_catvector)(const CatVector&, ...);	


namespace modules { namespace valvesurface {

void Init(vgui::ISurface* VSurface) { // To pass surface into the module, it requests it at init
	
	g_ISurface = VSurface; // Set the pointer passed from the game module into the draw functions
	
	// Pass the surface functions to the drawmodule manager
	drawmgr::InitLine(Line);
	drawmgr::InitRect(Rect);
	drawmgr::InitRectFilled(RectFilled);
	drawmgr::string::InitString(String1);
	drawmgr::string::InitStringLength(StringLength);
	
}
	
}} 
