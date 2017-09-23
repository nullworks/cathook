/*
 *
 *	This is a module for giving the cheat access to input information through the use of xlib.
 *	Exec the below to give the input manager acess to input info through this module!
 *		-Onee
 *
 */

#include "../../logging.h"				// Logging is cool!
#include "../../managers/inputmgr.hpp" 	// So we can pass it to the mgr
#include "xlibinput.hpp"				// So we have something to pass

#include "init.hpp"

namespace modules { namespace xlibinput {
	
void Init() {
	logging::Info("Initializing xlib input module...");
	
	CatUserInp.InitRefresh(Refresh);
	
	logging::Info("xlib input module initialized!");
}
	
}}