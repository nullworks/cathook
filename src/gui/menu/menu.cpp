
/*
 *
 * 	This is the menu constructor for the gui. This build a menu our of elements for us to use.
 *
 */

#include <vector>

#include "../gui.hpp"
#include "../../managers/inputmgr.hpp"		// So we get input
#include "../../util/catvars.hpp"	// Obs we need it
#include "../base/elements.hpp"		// Include elements for building a gui
#include "menutree.hpp"				// This holds our menu information for us to build from

#include "menu.hpp"

namespace gui { namespace menu {

// User vars
CatEnum gui_catenum({"Visuals", "Gui"});
CatVarBool menu_enabled(gui_catenum, "menu_visible", true, "Show Menu", "Should menu be visible?");
CatVarInt menu_x(gui_catenum, "menu_x", -20000, "Menu Location X", "Part of the position of the menu");
CatVarInt menu_y(gui_catenum, "menu_y", -20000, "Menu Location Y", "Part of the position of the menu");
CatVarInt menu_scale(gui_catenum, "menu_y", 100, "Menu Scaling", "Allows you to scale the menu.\n100 = 100% Scaling", 250);

// The root of cathooks menu
CBaseWidget* main_menu = nullptr;
// Used to store "where in the menu tree" we are
static int menu_location[2];
// aspect ratio, dont judge
const int aspectx = 12;
const int aspecty = 7;
		
// Clears the things owned by the menu and rebuilds it
void ReConstruct() {
	
	// Check if the main menu is created
	if (!main_menu) {
		main_menu = gui::element::RootCreate();
	}
	
	// Create a good size to the screen
	main_menu->height = (CatUserInp.boundsy * 0.45F) * (menu_scale * 0.01); // Make it 38% of the screen, and scale with scale var
	main_menu->width = (main_menu->height / aspecty) * aspectx;	// Extrapolate with the aspect ratio
		
	// Movement handler.
	if (main_menu->performed_last) {
		menu_x = main_menu->rootx;
		menu_y = main_menu->rooty;
	} else {
		// Get a center if menu is at default position
		if (menu_x == -20000 || menu_y == -20000) { 
			menu_x = (CatUserInp.boundsx * 0.5) - (main_menu->width * 0.5);
			menu_y = (CatUserInp.boundsy * 0.5) - (main_menu->height * 0.5);
		}
		// Set root to catvars
		main_menu->rootx = menu_x;
		main_menu->rooty = menu_y;
	}
	
	// Delete existing elements
	if (!main_menu->child_widgets.empty()) {
		for (CBaseWidget* widget : main_menu->child_widgets) {
			DeleteWidgetTree(widget);
		}
	}
	
	// Construct Menu Tree
	CMenuTree* menu_tree = GetMenuTree();
	//int top_barheight = main_menu->width * 0.105; // 10.5:1 aspect ratio is good for top bar i guess...
	for (CMenuTree* tree_branch : menu_tree->children) {
		// make buttons on top to change menu_location[0] to that value of the button, and menu_location[1] to 0
	}

	for (CMenuTree* tree_branch : menu_tree->children[menu_location[0]]->children) {
		// make buttons on top to change menu_location[1] to that value of the button
	}
	// Check if we dont have a position, if so, try to use default
	/*if (menu_location.empty()) {
		menu_location.push_back(0);	// Just use the first...
		
	}*/
	
	
	
	main_menu->color = GUIColor();
}

void Init() {

	
	
}
}}