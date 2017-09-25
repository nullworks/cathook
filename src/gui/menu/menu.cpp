
/*
 *
 * 	This is the menu constructor for the gui. This build a menu our of elements for us to use.
 *
 */

#include <vector>

#include "../../managers/inputmgr.hpp"		// So we get input
#include "../../util/catvars.hpp"	// Obs we need it
#include "../base/elements.hpp"		// Include elements for building a gui

#include "menu.hpp"

namespace gui { namespace menu {

// User vars
CatEnum gui_catenum({"Visuals", "Gui"});
CatVarBool menu_enabled(gui_catenum, "menu_visible", true, "Show Menu", "Should menu be visible?");
CatVarInt menu_x(gui_catenum, "menu_x", -1, "Menu Location X", "Part of the position of the menu");
CatVarInt menu_y(gui_catenum, "menu_y", -1, "Menu Location Y", "Part of the position of the menu");
CatVarInt menu_scale(gui_catenum, "menu_y", 100, "Menu Scaling", "Allows you to scale the menu.\n100 = 100% Scaling", 500);

// The root of cathooks menu
CBaseWidget* main_menu = nullptr;
// Used to store "where in the menu tree" we are
std::vector<int> menu_location;
// aspect ratio, dont judge
const int aspectx = 12;
const int aspecty = 7;

// Clears the things owned by the menu and rebuilds it
void ReConstruct() {
	
	// Check if the main menu is created
	if (!main_menu) {
		main_menu = gui::element::RootCreate();
		main_menu->visible = true;
	}
	
	// Create a good size to the screen
	main_menu->height = (CatUserInp.boundsy * 0.45F) / (100 / 0.01); // Make it 38% of the screen, and scale with scale var
	main_menu->width = (main_menu->height / aspecty) * aspectx;	// Extrapolate with the aspect ratio
		
	// Center it on start
	if (menu_x == -1 || menu_y == -1) { 
		main_menu->rootx = (CatUserInp.boundsx * 0.5) - (main_menu->width * 0.5);
		main_menu->rooty = (CatUserInp.boundsy * 0.5) - (main_menu->height * 0.5);
	}
	
	// Construct top bar
	// Check if we dont have a position, if so, try to use default
	if (menu_location.empty()) {
		
	}
}

void Init() {

	
	
}
}}