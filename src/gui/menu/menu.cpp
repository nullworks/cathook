
/*
 *
 * 	This is the menu constructor for the gui. This build a menu our of elements for us to use.
 *
 */

#include "../../util/catvars.hpp"
#include "../base/elements.hpp"		// Include elements for building a gui

#include "menu.hpp"

namespace gui { namespace menu {

// aspect ratio, i like 11:7
const int aspectx = 11;
const int aspecty = 7;

// The root of cathooks menu
CBaseWidget* main_menu = nullptr;
CBaseWidget* main_menu2 = nullptr;

CatEnum gui_catenum({"Visuals", "Gui"});
CatVarBool menu_enabled(gui_catenum, "menu_visible", true, "Show Menu", "Should menu be visible?");
	
void Construct() {
	if (!main_menu) {
		main_menu = gui::element::RootCreate();
		main_menu->rootx = 400;
		main_menu->rooty = 400;
		main_menu->widthx = 400;
		main_menu->widthy = 250;
		main_menu->visible = true;
	}
	if (!main_menu2) {
		main_menu2 = gui::element::RootCreate();
		main_menu2->rootx = 600;
		main_menu2->rooty = 600;
		main_menu2->widthx = 400;
		main_menu2->widthy = 250;
		main_menu2->visible = true;
	}
}
void Init() {

	
	
}
}}